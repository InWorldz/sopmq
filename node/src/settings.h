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

#ifndef __sopmq__settings__
#define __sopmq__settings__

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/noncopyable.hpp>

#include <string>
#include <vector>
#include <cstdint>

namespace bmp = boost::multiprecision;

///
/// SOPMQ
///
namespace sopmq {
    
    ///
    /// Namespace for the sopmq server
    ///
    namespace node {
        
        ///
        /// Settings singleton for a sopmq node
        ///
        class settings : public boost::noncopyable
        {
        public:
            ///
            /// The default maximum message size
            ///
            static const uint32_t DEFAULT_MAX_MESSAGE_SIZE;

            ///
            /// Returns our singleton instance
            ///
            static settings& instance();
            
        public:
            ///
            /// The beginning of the range we're in charge of
            ///
            bmp::uint128_t range;
            
            ///
            /// The IP address to bind to
            ///
            std::string bindAddress;
            
            ///
            /// The port to bind the node to
            ///
            uint16_t port;
            
            ///
            /// IP adresses of cassandra nodes we should connect to for reading
            /// and writing data
            ///
            std::vector<std::string> cassandraSeeds;
            
            ///
            /// Seed nodes we should initiate our initial connection to
            ///
            std::vector<std::string> mqSeeds;
            
            ///
            /// The maximum message size that we allow
            ///
            uint32_t maxMessageSize;
            
            
        private:
            settings();
            ~settings();
        };
        
    }
}

#endif /* defined(__sopmq__settings__) */
