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

#ifndef __sopmq__endpoint__
#define __sopmq__endpoint__

#include <boost/asio.hpp>

#include <string>
#include <exception>
#include <ostream>

namespace sopmq {
    namespace shared {
        namespace net {
            
            ///
            /// The endpoint for a node
            ///
            class endpoint
            {
            public:
                ///
                /// The endpoint protocol
                ///
                enum endpoint_proto
                {
                    SOPMQvUNKNOWN,
                    SOPMQv1
                };
                
            public:
                //endpoint() = default;
                
                ///
                /// Constructs an endpoint with an as of yet unknown address
                ///
                endpoint();
                
                ///
                /// Constructs an endpoint by parsing a uri
                ///
                explicit endpoint(const std::string& uri);
                
                ///
                /// Constructs an endpoint from a boost endpoint
                /// (assumes protocol v1)
                ///
                explicit endpoint(const boost::asio::ip::tcp::endpoint& ep);
                
                ///
                /// Constructs an endpoint given a hostname and a port
                ///
                endpoint(const std::string& hostName, unsigned short port);
                
                ///
                /// dtor
                ///
                virtual ~endpoint();

                
                ///
                /// The host name for this endpoint
                ///
                const std::string& host_name() const;
                
                ///
                /// The port for this endpoint
                ///
                unsigned short port() const;
                
                ///
                /// The protocol this endpoint speaks
                ///
                endpoint_proto protocol() const;
                
                ///
                /// Returns a string/URI representation of this endpoint
                ///
                const std::string& str() const;
                
            private:
                std::string _hostname;
                unsigned short _port;
                endpoint_proto _proto;
                
                std::string _stringUri;
                
                ///
                /// Internal function to parse an input URI
                ///
                void parse_uri(const std::string& uri);
                
                ///
                /// Returns the tcp/ip port that is the default for the given protocol
                ///
                unsigned short get_default_port_for_proto(endpoint_proto proto) const;
                
                ///
                /// Returns the protocol that is assciated with the given scheme name
                ///
                endpoint_proto scheme_name_to_protocol(const std::string& scheme) const;
            };
            
            ///
            /// Comparison to allow EPs to be used in ordered collections
            ///
            bool operator < (const endpoint& lhs, const endpoint& rhs);
            
            ///
            /// For logging
            ///
            std::ostream& operator<< (std::ostream&, const endpoint&);
            
            ///
            /// Error thrown when the URI fed to the endpoint constructor is invalid
            ///
            class uri_parse_error : public std::runtime_error
            {
            public:
                uri_parse_error(const std::string& message) : std::runtime_error(message) {}
            };
            
            
            ///
            /// Error thrown when the protocol specified in a URI is invalid
            ///
            class invalid_protocol_error : public std::runtime_error
            {
            public:
                invalid_protocol_error(const std::string& message) : std::runtime_error(message) {}
            };
            
        }
    }
}

#endif /* defined(__sopmq__endpoint__) */
