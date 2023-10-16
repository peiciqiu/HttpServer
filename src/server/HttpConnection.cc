/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <cstdint>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>
#include <iostream>      


#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace searchserver {

static const char *kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::next_request(HttpRequest *request) {
  // Use "wrapped_read" to read data into the buffer_
  // instance variable.  Keep reading data until either the
  // connection drops or you see a "\r\n\r\n" that demarcates
  // the end of the request header.
  //
  // Once you've seen the request header, use parse_request()
  // to parse the header into the *request argument.
  //
  // Very tricky part:  clients can send back-to-back requests
  // on the same socket.  So, you need to preserve everything
  // after the "\r\n\r\n" in buffer_ for the next time the
  // caller invokes next_request()!

  // TODO: implement
  
  std::string buffer;
  buffer += buffer_;
  buffer_.clear();
  int index = -1;
  int byte = 0;

  while(byte != -1){
    index = buffer.find("\r\n\r\n");
    if(index != -1) {break;}
    byte = wrapped_read(fd_, &buffer);
  }
  if(byte == -1 && index == -1){return false;}

  string requestHeader = buffer.substr(0, index);
  buffer_ = buffer.substr(index + kHeaderEndLen);
  bool success = parse_request(requestHeader, request);
  return success;
}

bool HttpConnection::write_response(const HttpResponse &response) {
  // Implement so that the response is converted to a string
  // and written out to the socket for this connection

  // TODO: implement
  std::string responseString = response.GenerateResponseString();
  int byte = wrapped_write(fd_, responseString);
  return byte == responseString.length();
}

bool HttpConnection::parse_request(const string &request, HttpRequest* out) {
  HttpRequest req("/");  // by default, get "/".

  // Split the request into lines.  Extract the URI from the first line
  // and store it in req.URI.  For each additional line beyond the
  // first, extract out the header name and value and store them in
  // req.headers_ (i.e., HttpRequest::AddHeader).  You should look
  // at HttpRequest.h for details about the HTTP header format that
  // you need to parse.
  //
  // You'll probably want to look up boost functions for (a) splitting
  // a string into lines on a "\r\n" delimiter, (b) trimming
  // whitespace from the end of a string, and (c) converting a string
  // to lowercase.
  //
  // If a request is malformed, return false, otherwise true and 
  // the parsed request is returned via *out
  // TODO: implement

  vector<string> lines;
  boost::split(lines, request, boost::is_any_of("\r\n"), boost::token_compress_on);
  if(lines.size() < 1) {return false;}

  vector<string> firstline;
  boost::split(firstline,  lines[0], boost::is_any_of(" "), boost::token_compress_on);
  if(firstline.size() < 2) {return false;}
  string uri = boost::to_lower_copy(firstline[1]);
  out -> set_uri (uri);

  for(int i = 1; i < lines.size(); i++){
    string currLine = boost::to_lower_copy(lines[i]);
    size_t index = currLine.find(":");
    if (index == string::npos) continue;
    string header = currLine.substr(0, index);
    string value = currLine.substr(index + 2);
    out -> AddHeader(header, value);
  }

  return true;
}

}  // namespace searchserver
