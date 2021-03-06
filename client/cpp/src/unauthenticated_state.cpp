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

#include "unauthenticated_state.h"
#include "session.h"
#include "logging.h"
#include "ChallengeResponseMessage.pb.h"
#include "AnswerChallengeMessage.pb.h"
#include "AuthAckMessage.pb.h"
#include "GetChallengeMessage.pb.h"
#include "util.h"
#include "messageutil.h"
#include "network_operation_result.h"

#include <functional>
#include <boost/assert.hpp>
#include <cryptopp/sha.h>

using sopmq::message::message_dispatcher;
using namespace std::placeholders;
using sopmq::shared::util;
using sopmq::message::messageutil;

namespace sopmq {
    namespace client {
        namespace impl {
            
            unauthenticated_state::unauthenticated_state(cluster_connection::ptr conn, session::wptr session,
                                                         const std::string& username, const std::string& password,
                                                         std::function<void(bool)> authCallback)
            : _connection(conn), _session(session), _username(username), _password(password), _authCallback(authCallback),
            _dispatcher(std::bind(&unauthenticated_state::on_unhandled_message,
                                  this, _1, _2))
            {
            }
            
            unauthenticated_state::~unauthenticated_state()
            {
                LOG_SRC(debug) << "~unauthenticated_state()";
            }
            
            void unauthenticated_state::state_entry()
            {
                LOG_SRC(debug) << "unauthenticated_state::state_entry()";
                
                //send a request to the server to get an auth challenge
                GetChallengeMessage_ptr gcm = messageutil::make_message<GetChallengeMessage>(_connection->get_next_id(), 0);
                gcm->set_type(GetChallengeMessage::CLIENT);
                
                //set the dispatcher to catch the reply
                std::function<void(const sopmq::shared::net::network_operation_result&, ChallengeResponseMessage_ptr)> func
                    = std::bind(&unauthenticated_state::on_challenge_response, this, _1, _2);
                
                _dispatcher.set_handler(func, gcm->identity().id());
                
                _connection->send_message(message::MT_GET_CHALLENGE, gcm,
                                          std::bind(&unauthenticated_state::on_message_sent, shared_from_this(), _1));
                
                _connection->read_message(_dispatcher, std::bind(&unauthenticated_state::on_message_received, shared_from_this(), _1));
            }
            
            void unauthenticated_state::on_message_sent(const shared::net::network_operation_result& result)
            {
                if (!result.was_successful())
                {
                    //auth failed
                    LOG_SRC(error) << _connection->endpoint()
                        << " network error during session authorization: "
                        << result.get_error().what();
                    
                    _authCallback(false);
                }
            }
            
            void unauthenticated_state::on_message_received(const shared::net::network_operation_result& result)
            {
                if (!result.was_successful())
                {
                    //auth failed
                    LOG_SRC(error) << _connection->endpoint()
                        << " network error during session authorization: "
                        << result.get_error().what();
                    
                    _authCallback(false);
                }
            }
            
            void unauthenticated_state::on_unhandled_message(Message_ptr message, const std::string& typeName)
            {
                LOG_SRC(error) << _connection->endpoint()
                    << " protocol violation: unexpected message: "
                    << typeName;
                
                if (auto session = _session.lock())
                {
                    session->protocol_violation();
                }
            }
            
            void unauthenticated_state::on_challenge_response(const sopmq::shared::net::network_operation_result&, ChallengeResponseMessage_ptr response)
            {
                if (!response) return; //error
                
                BOOST_ASSERT(response->has_challenge());
                
                //respond to the challenge with:
                //sha256([username]) + " " + sha256(sha256([password])[challenge])
                unsigned char hashResult[CryptoPP::SHA256::DIGESTSIZE];
                
                CryptoPP::SHA256 sha;
                
                sha.CalculateDigest(&hashResult[0], (unsigned char*)_username.c_str(), _username.length());
                std::string unameHash = util::hex_encode(hashResult, CryptoPP::SHA256::DIGESTSIZE);
                
                sha.Restart();
                
                sha.CalculateDigest(&hashResult[0], (unsigned char*)_password.c_str(), _password.length());
                std::string pwHash = util::hex_encode(hashResult, CryptoPP::SHA256::DIGESTSIZE);
                
                const std::string& challenge = response->challenge();
                auto pwHashAndChallenge = pwHash + challenge;
                
                sha.Restart();
                
                sha.CalculateDigest(&hashResult[0], (unsigned char*)pwHashAndChallenge.c_str(), pwHashAndChallenge.length());
                std::string result = util::hex_encode(hashResult, CryptoPP::SHA256::DIGESTSIZE);
                
                //clear the handler for the challenge response since we're not looking for that anymore
                _dispatcher.set_handler(std::function<void(const sopmq::shared::net::network_operation_result&,ChallengeResponseMessage_ptr)>());

                //generate the answer
                AnswerChallengeMessage_ptr acm = messageutil::make_message<AnswerChallengeMessage>(_connection->get_next_id(), response->identity().id());
                acm->set_uname_hash(unameHash);
                acm->set_challenge_response(result);
                
                //set the handler for the AuthAck
                std::function<void(const sopmq::shared::net::network_operation_result&, AuthAckMessage_ptr)> func
                    = std::bind(&unauthenticated_state::on_auth_ack, this, _1, _2);
                _dispatcher.set_handler(func, acm->identity().id());
                
                
                _connection->send_message(message::MT_ANSWER_CHALLENGE, acm,
                                          std::bind(&unauthenticated_state::on_message_sent, shared_from_this(), _1));
                
                _connection->read_message(_dispatcher, std::bind(&unauthenticated_state::on_message_received, shared_from_this(), _1));
            }
            
            void unauthenticated_state::on_auth_ack(const sopmq::shared::net::network_operation_result&, AuthAckMessage_ptr response)
            {
                if (!response) return; //error
                
                if (response->has_authorized() && response->authorized())
                {
                    _authCallback(true);
                }
                else
                {
                    //auth failed
                    LOG_SRC(error) << _connection->endpoint() << " session authorization denied";
                    _authCallback(false);
                }
            }
            
            void unauthenticated_state::publish_message(const std::string& queueId, bool storeIfCantPipe, int ttl,
                                                        const std::string& data, publish_message_callback callback)
            {
                throw std::logic_error("Call to publish_message() is invalid when the session is unauthenticated");
            }
            
        }
    }
}