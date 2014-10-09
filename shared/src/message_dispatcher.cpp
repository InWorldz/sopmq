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

#include "message_dispatcher.h"

//
// ATTENTION: This file uses cog (http://nedbatchelder.com/code/cog/) for code
// generation. Do not edit the generated chunks between cog and end tags
//


/*[[[cog
 import cog
 import glob
 import os
 
 ast = '*'
 
 first_lower = lambda s: s[:1].lower() + s[1:] if s else ''
 fnames = glob.glob("../messages/" + ast + ".proto")
 ]]]*/
//[[[end]]]

/*[[[cog
 for fn in fnames:
   rawname = os.path.splitext(os.path.basename(fn))[0]
   cog.outl("#include \"%s.pb.h\"" % rawname)
 ]]]*/
#include "AnswerChallengeMessage.pb.h"
#include "AuthAckMessage.pb.h"
#include "ChallengeResponseMessage.pb.h"
#include "ConsumeFromQueueMessage.pb.h"
#include "ConsumeResponseMessage.pb.h"
#include "GetChallengeMessage.pb.h"
#include "GossipMessage.pb.h"
#include "GossipNodeData.pb.h"
#include "Identifier.pb.h"
#include "NodeClock.pb.h"
#include "PublishMessage.pb.h"
#include "PublishResponseMessage.pb.h"
#include "VectorClock.pb.h"
//[[[end]]]

namespace sopmq {
    namespace message {
        
        message_dispatcher::message_dispatcher(std::function<void(Message_ptr, const std::string&)> unhandledHandler)
        : _unhandledHandler(unhandledHandler)
        {
            
        }
        
        message_dispatcher::~message_dispatcher()
        {
            
        }
        
        /*[[[cog
         for fn in fnames:
           
           rawname = os.path.splitext(os.path.basename(fn))[0]
           cog.outl("")
           cog.outl("void message_dispatcher::dispatch(%s_ptr %s)" % (rawname,first_lower(rawname)))
           cog.outl("{")
           cog.outl("    do_dispatch(_%sHandler, %s);" % (first_lower(rawname), first_lower(rawname)))
           cog.outl("}")
           cog.outl("")
         ]]]*/

        void message_dispatcher::dispatch(AnswerChallengeMessage_ptr answerChallengeMessage)
        {
            do_dispatch(_answerChallengeMessageHandler, answerChallengeMessage);
        }


        void message_dispatcher::dispatch(AuthAckMessage_ptr authAckMessage)
        {
            do_dispatch(_authAckMessageHandler, authAckMessage);
        }


        void message_dispatcher::dispatch(ChallengeResponseMessage_ptr challengeResponseMessage)
        {
            do_dispatch(_challengeResponseMessageHandler, challengeResponseMessage);
        }


        void message_dispatcher::dispatch(ConsumeFromQueueMessage_ptr consumeFromQueueMessage)
        {
            do_dispatch(_consumeFromQueueMessageHandler, consumeFromQueueMessage);
        }


        void message_dispatcher::dispatch(ConsumeResponseMessage_ptr consumeResponseMessage)
        {
            do_dispatch(_consumeResponseMessageHandler, consumeResponseMessage);
        }


        void message_dispatcher::dispatch(GetChallengeMessage_ptr getChallengeMessage)
        {
            do_dispatch(_getChallengeMessageHandler, getChallengeMessage);
        }


        void message_dispatcher::dispatch(GossipMessage_ptr gossipMessage)
        {
            do_dispatch(_gossipMessageHandler, gossipMessage);
        }


        void message_dispatcher::dispatch(GossipNodeData_ptr gossipNodeData)
        {
            do_dispatch(_gossipNodeDataHandler, gossipNodeData);
        }


        void message_dispatcher::dispatch(Identifier_ptr identifier)
        {
            do_dispatch(_identifierHandler, identifier);
        }


        void message_dispatcher::dispatch(NodeClock_ptr nodeClock)
        {
            do_dispatch(_nodeClockHandler, nodeClock);
        }


        void message_dispatcher::dispatch(PublishMessage_ptr publishMessage)
        {
            do_dispatch(_publishMessageHandler, publishMessage);
        }


        void message_dispatcher::dispatch(PublishResponseMessage_ptr publishResponseMessage)
        {
            do_dispatch(_publishResponseMessageHandler, publishResponseMessage);
        }


        void message_dispatcher::dispatch(VectorClock_ptr vectorClock)
        {
            do_dispatch(_vectorClockHandler, vectorClock);
        }

        //[[[end]]]
        
        /*[[[cog
         for fn in fnames:
           rawname = os.path.splitext(os.path.basename(fn))[0]
           cog.outl("")
           cog.outl("void message_dispatcher::set_handler(std::function<void(%s_ptr)> handler)" % rawname)
           cog.outl("{")
           cog.outl("    _%sHandler = handler;" % first_lower(rawname))
           cog.outl("}")
           cog.outl("")
         ]]]*/

        void message_dispatcher::set_handler(std::function<void(AnswerChallengeMessage_ptr)> handler)
        {
            _answerChallengeMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(AuthAckMessage_ptr)> handler)
        {
            _authAckMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(ChallengeResponseMessage_ptr)> handler)
        {
            _challengeResponseMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(ConsumeFromQueueMessage_ptr)> handler)
        {
            _consumeFromQueueMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(ConsumeResponseMessage_ptr)> handler)
        {
            _consumeResponseMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(GetChallengeMessage_ptr)> handler)
        {
            _getChallengeMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(GossipMessage_ptr)> handler)
        {
            _gossipMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(GossipNodeData_ptr)> handler)
        {
            _gossipNodeDataHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(Identifier_ptr)> handler)
        {
            _identifierHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(NodeClock_ptr)> handler)
        {
            _nodeClockHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(PublishMessage_ptr)> handler)
        {
            _publishMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(PublishResponseMessage_ptr)> handler)
        {
            _publishResponseMessageHandler = handler;
        }


        void message_dispatcher::set_handler(std::function<void(VectorClock_ptr)> handler)
        {
            _vectorClockHandler = handler;
        }

        //[[[end]]]
    }
}

