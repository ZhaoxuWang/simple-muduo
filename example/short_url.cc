#include "http/HttpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/EventLoop.h"
#include "net/utils.h"
#include "http/ShortURL.h"

#include <iostream>
#include <map>

std::map<std::string, std::string> redirections;
bool benchmark = true;

void onRequest(const HttpRequest& req, HttpResponse* resp){
    std::cout << "Headers " << req.methodString() << " " << req.path() << std::endl;

    if (!benchmark)
    {
        const std::map<std::string, std::string>& headers = req.headers();
        for (const auto& header : headers)
        {
        std::cout << header.first << ": " << header.second << std::endl;
        }
    }
    if(!req.query().empty()){
        if(req.path() == "/set"){
            std::string mixURL = ShortURL::get_shortURL(req.query());
            std::string short_url;
            for(int i = 0; i < 4; i++){
                if(redirections.find(mixURL.substr(6 * i, 6)) != redirections.end()){
                    if(redirections[mixURL.substr(6 * i, 6)] == req.query()){
                        short_url = mixURL.substr(6 * i, 6);
                        break;
                    }
                }
                else{
                    short_url = mixURL.substr(6 * i, 6);
                    break;
                }
            }

            resp->setStatusCode(HttpResponse::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/html");
            if(short_url.empty()){
                resp->setBody("<html><head><title>This is title</title></head>"
                    "<body><h1>Hello</h1>long url is " + req.query() + "<br>"
                    "fail to resolve" + 
                    "</body></html>");
            }
            else if(redirections.find(short_url) != redirections.end()){
                resp->setBody("<html><head><title>This is title</title></head>"
                    "<body><h1>Hello</h1>long url is " + req.query() + "<br>"
                    "already saved: " + short_url + 
                    "</body></html>");
            }
            else{
                redirections[short_url] = req.query();
                resp->setBody("<html><head><title>This is title</title></head>"
                    "<body><h1>Hello</h1>long url is " + req.query() + "<br>"
                    "shortURL is " + short_url + 
                    "</body></html>");
            }

            
        }
        else{
            resp->setStatusCode(HttpResponse::k400BadRequest);
            resp->setStatusMessage("Bad Request");
            resp->setCloseConnection(true);
        }
        
    }
    else{
        std::map<std::string, std::string>::const_iterator it = redirections.find(req.path().substr(1, req.path().size()-1));

        if (it != redirections.end()){
            resp->setStatusCode(HttpResponse::k301MovedPermanently);
            resp->setStatusMessage("Moved Permanently");
            resp->addHeader("Location", it->second);
            resp->setCloseConnection(true);
        }
        else{
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }
    }
    
}

int main(int argc, char* argv[])
{   
  int numThreads = 0;
  if (argc > 1)
  {
    benchmark = true;
    numThreads = atoi(argv[1]);
  }
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8001), "dummy");
  server.setHttpCallback(onRequest);
  server.setThreadNum(numThreads);
  server.start();
  loop.loop();
}