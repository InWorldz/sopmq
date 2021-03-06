/*
 * SOPMQ - Scalable optionally persistent message queue
 * Copyright 2014 InWorldz, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "csauthenticated.h"

#include "messageutil.h"
#include "network_error.h"
#include "settings.h"
#include "logging.h"
#include "unavailable_error.h"
#include "message_types.h"
#include "util.h"
#include "quorum_logic.h"

#include "PublishMessage.pb.h"
#include "PublishResponseMessage.pb.h"
#include "ProxyPublishResponseMessage.pb.h"

#include <functional>

using namespace std::placeholders;

using sopmq::error::network_error;
using sopmq::error::unavailable_error;
using sopmq::message::messageutil;
using sopmq::node::settings;

namespace sopmq {
    namespace node {
        namespace connection {
            
            csauthenticated::csauthenticated(boost::asio::io_service& ioService, connection_in::ptr conn,
                const ring& ring)
            : _ioService(ioService), _conn(conn), _ring(ring),
            _dispatcher(std::bind(&csauthenticated::unhandled_message, this, _1))
            {
                LOG_SRC(debug) << "csauthenticated()";
            }
            
            csauthenticated::~csauthenticated()
            {
                
            }
            
            void csauthenticated::start()
            {
                //set us up to handle any messages that clients should be sending to us
                std::function<void(const shared::net::network_operation_result&,PublishMessage_ptr)> func
                    = std::bind(&csauthenticated::handle_post_message, this, _1, _2);
                
                _dispatcher.set_handler(func);
                
                _conn->read_message(_dispatcher, std::bind(&csauthenticated::handle_read_result, shared_from_this(), _1));
            }
            
            void csauthenticated::unhandled_message(Message_ptr message)
            {
                _conn->handle_error(network_error("Unexpected message received during operation"));
            }
            
            void csauthenticated::handle_read_result(const shared::net::network_operation_result& result)
            {
                if (!result.was_successful())
                {
                    _conn->handle_error(result.get_error());
                }
                else
                {
                    _conn->read_message(_dispatcher, std::bind(&csauthenticated::handle_read_result, shared_from_this(), _1));
                }
            }

            void csauthenticated::handle_write_result(const shared::net::network_operation_result& result)
            {
                if (!result.was_successful())
                {
                    _conn->handle_error(result.get_error());
                }
            }
            
            void csauthenticated::handle_post_message(const shared::net::network_operation_result& result, PublishMessage_ptr message)
            {
                LOG_SRC(debug) << "handle_post_message(): result: " << (result.was_successful() ? "true" : "false");
                if (! result.was_successful()) return;
                
                try
                {
                    std::array<node::ptr, 3> nodes = _ring.find_quorum_for_operation(sopmq::shared::util::murmur_hash3(message->queue_id()));
                    
                    class context
                    {
                    public:
                        std::vector<vector_clock3> clocks;
                    };
                    
                    quorum_logic<3, context>::ptr logic = std::make_shared<quorum_logic<3, context> >(nodes);
                    
                    // if we can not successfully message a quorum of nodes, this will fire off the failure
                    logic->set_fail_function([=] {
                        PublishResponseMessage_ptr response
                            = messageutil::make_message<PublishResponseMessage>(_conn->get_next_id(), message->identity().id());
                        
                        response->set_status(PublishResponseMessage_Status_UNAVAILABLE);
                        
                        _conn->send_message(sopmq::message::MT_PUBLISH_RESPONSE, response,
                                            std::bind(&csauthenticated::handle_write_result, shared_from_this(), _1));
                    });
                    
                    logic->set_function([=](node::ptr node) {
                        
                        node->operations().send_proxy_publish(message, [=](intra::operation_result<ProxyPublishResponseMessage_ptr> result){
                            
                            try
                            {
                                result.rethrow_error();
                                
                                if (result.message()->status() == ProxyPublishResponseMessage_Status_QUEUED)
                                {
                                    logic->node_success(node);
                                    logic->ctx().clocks.push_back(result.message()->clock());
                                    
                                    if (logic->operation_succeeded())
                                    {
                                        //we have the result from all nodes, combine and send the message stamp
                                        vector_clock3 maxClock = vector_clock3::max(logic->ctx().clocks[0], logic->ctx().clocks[1]);
                                        
                                        //tell the quorum the resulting message ID
                                        this->do_stamp_message(logic->successful_nodes(), message, maxClock);
                                    }
                                }
                                else
                                {
                                    LOG_SRC(warning)
                                        << "send_proxy_publish(): node "
                                        << node->node_id() << " failed with status "
                                        << result.message()->status();
                                    
                                    logic->node_failed(node);
                                }
                            }
                            catch (const comparison_error& e) //issue with the size of the network vector clocks
                            {
                                LOG_SRC(warning)
                                    << "send_proxy_publish(): node "
                                    << node->node_id() << " failed with error "
                                    << e.what();
                                    
                                logic->node_failed(node);
                            }
                            catch (const std::runtime_error& e)
                            {
                                LOG_SRC(warning)
                                    << "send_proxy_publish(): node "
                                    << node->node_id() << " failed with error "
                                    << e.what();
                                    
                                logic->node_failed(node);
                            }
                        });
                    });

                    
                    logic->run();
                }
                catch (const unavailable_error& e)
                {
                    LOG_SRC(error) << "A quorum could not be reached for PUBLISH to " << message->queue_id();

                    PublishResponseMessage_ptr response 
                        = messageutil::make_message<PublishResponseMessage>(_conn->get_next_id(), message->identity().id());

                    response->set_status(PublishResponseMessage_Status_UNAVAILABLE);

                    _conn->send_message(sopmq::message::MT_PUBLISH_RESPONSE, response, 
                        std::bind(&csauthenticated::handle_write_result, shared_from_this(), _1));
                }
            }
            
            void csauthenticated::do_stamp_message(std::vector<node::ptr>& nodes, PublishMessage_ptr message, const vector_clock3 &maxClock)
            {
                
            }
            
            std::string csauthenticated::get_description() const
            {
                return "csauthenticated";
            }
        }
    }
}