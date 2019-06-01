#include "application.hpp"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    uint16_t port            = 4000;
    std::string  threads     = "";
    unsigned int concurrency = 0;
    
    po::options_description description("Available options");
    description.add_options()
        ( "help,h",                                       "show this help message"                            )
        ( "port,p",    po::value<uint16_t>(&port),        "port identifier"                                   )
        ( "threads,t", po::value<std::string>(&threads),  "number of threads of execution (0 for autodetect)" )
        ;

    po::positional_options_description pos_description;
    pos_description.add("port", -1);
    
    try
    {
        po::variables_map vm;
        po::store(
            po::command_line_parser(argc, argv)
                .options(description)
                .positional(pos_description)
                .run()
          , vm);
        
        po::notify(vm);
        
        if (vm.count("help") != 0)
        {
            throw po::error("");
        }
        else if (vm.count("port") == 0)
        {
            throw po::error("Port identifier must be specified");
        }

        if (vm.count("threads") == 0)
        {
            concurrency = 1;
        }
        else
        {
            try
            {
                concurrency = boost::lexical_cast<unsigned int>(threads);
            }
            catch(...)
            {
                // Failure is to be expected here, since it might be an empty
                // string.  In this case, concurrency will be a detectable 0.
            }
            
            if (concurrency == 0)
            {
                concurrency = std::thread::hardware_concurrency();
            }

            // According to the Boost docs, "thread::hardware_concurrency()" 
            // may return 0 on platforms that don't have information available
            // about cores/hyperthreading units, etc.  In this case, we will
            // default to one thread.
            if (concurrency == 0)
            {
                concurrency = 1;
            }
        }
    }
    catch(po::error &err)
    {
        if (strlen(err.what()) == 0)
        {
            std::cout << boost::format("USAGE: %s <port identifier>|<options>\n")
                        % argv[0]
                 << description
                 << std::endl;
                 
            return EXIT_SUCCESS;
        }
        else
        {
            std::cerr << boost::format("ERROR: %s\n\nUSAGE: %s <port identifier>|<options>\n")
                        % err.what()
                        % argv[0]
                 << description
                 << std::endl;
        }
        
        return EXIT_FAILURE;
    }

    ma::application application{port};

    std::vector<std::thread> threadpool;

    for (unsigned int thr = 0; thr < concurrency; ++thr)
    {
        threadpool.emplace_back([&application]{application.run();});
    }
    
    for (auto &pthread : threadpool)
    {
        pthread.join();
    }
    
    return EXIT_SUCCESS;
}
