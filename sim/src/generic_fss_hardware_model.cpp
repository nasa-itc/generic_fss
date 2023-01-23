#include <generic_fss_hardware_model.hpp>

namespace Nos3
{
    REGISTER_HARDWARE_MODEL(Generic_fssHardwareModel,"GENERIC_FSS");

    extern ItcLogger::Logger *sim_logger;

    Generic_fssHardwareModel::Generic_fssHardwareModel(const boost::property_tree::ptree& config) : SimIHardwareModel(config), _enabled(0)
    {
        /* Get the NOS engine connection string */
        std::string connection_string = config.get("common.nos-connection-string", "tcp://127.0.0.1:12001"); 
        sim_logger->info("Generic_fssHardwareModel::Generic_fssHardwareModel:  NOS Engine connection string: %s.", connection_string.c_str());

        /* Get a data provider */
        std::string dp_name = config.get("simulator.hardware-model.data-provider.type", "GENERIC_FSS_PROVIDER");
        _generic_fss_dp = SimDataProviderFactory::Instance().Create(dp_name, config);
        sim_logger->info("Generic_fssHardwareModel::Generic_fssHardwareModel:  Data provider %s created.", dp_name.c_str());

        /* Get on a protocol bus */
        /* Note: Initialized defaults in case value not found in config file */
        std::string bus_name = "usart_29";
        int node_port = 29;
        if (config.get_child_optional("simulator.hardware-model.connections")) 
        {
            /* Loop through the connections for hardware model */
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("simulator.hardware-model.connections"))
            {
                /* v.second is the child tree (v.first is the name of the child) */
                if (v.second.get("type", "").compare("usart") == 0)
                {
                    /* Configuration found */
                    bus_name = v.second.get("bus-name", bus_name);
                    node_port = v.second.get("node-port", node_port);
                    break;
                }
            }
        }
        _uart_connection.reset(new NosEngine::Uart::Uart(_hub, config.get("simulator.name", "generic_fss_sim"), connection_string, bus_name));
        _uart_connection->open(node_port);
        sim_logger->info("Generic_fssHardwareModel::Generic_fssHardwareModel:  Now on UART bus name %s, port %d.", bus_name.c_str(), node_port);
    
        /* Configure protocol callback */
        _uart_connection->set_read_callback(std::bind(&Generic_fssHardwareModel::uart_read_callback, this, std::placeholders::_1, std::placeholders::_2));

        /* Get on the command bus*/
        std::string time_bus_name = "command";
        if (config.get_child_optional("hardware-model.connections")) 
        {
            /* Loop through the connections for the hardware model */
            BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("hardware-model.connections"))
            {
                /* v.first is the name of the child */
                /* v.second is the child tree */
                if (v.second.get("type", "").compare("time") == 0) // 
                {
                    time_bus_name = v.second.get("bus-name", "command");
                    /* Found it... don't need to go through any more items*/
                    break; 
                }
            }
        }
        _time_bus.reset(new NosEngine::Client::Bus(_hub, connection_string, time_bus_name));
        sim_logger->info("Generic_fssHardwareModel::Generic_fssHardwareModel:  Now on time bus named %s.", time_bus_name.c_str());

        /* Construction complete */
        sim_logger->info("Generic_fssHardwareModel::Generic_fssHardwareModel:  Construction complete.");
    }


    Generic_fssHardwareModel::~Generic_fssHardwareModel(void)
    {        
        /* Close the protocol bus */
        _uart_connection->close();

        /* Clean up the data provider */
        delete _generic_fss_dp;
        _generic_fss_dp = nullptr;

        /* The bus will clean up the time node */
    }


    /* Automagically set up by the base class to be called */
    void Generic_fssHardwareModel::command_callback(NosEngine::Common::Message msg)
    {
        /* Get the data out of the message */
        NosEngine::Common::DataBufferOverlay dbf(const_cast<NosEngine::Utility::Buffer&>(msg.buffer));
        sim_logger->info("Generic_fssHardwareModel::command_callback:  Received command: %s.", dbf.data);

        /* Do something with the data */
        std::string command = dbf.data;
        std::string response = "Generic_fssHardwareModel::command_callback:  INVALID COMMAND! (Try HELP)";
        boost::to_upper(command);
        if (command.compare("HELP") == 0) 
        {
            response = "Generic_fssHardwareModel::command_callback: Valid commands are HELP, ENABLE, DISABLE, STATUS=X, or STOP";
        }
        else if (command.compare("ENABLE") == 0) 
        {
            _enabled = GENERIC_FSS_SIM_SUCCESS;
            response = "Generic_fssHardwareModel::command_callback:  Enabled";
        }
        else if (command.compare("DISABLE") == 0) 
        {
            _enabled = GENERIC_FSS_SIM_ERROR;
            response = "Generic_fssHardwareModel::command_callback:  Disabled";
        }
        else if (command.compare("STOP") == 0) 
        {
            _keep_running = false;
            response = "Generic_fssHardwareModel::command_callback:  Stopping";
        }
        /* TODO: Add anything additional commands here */

        /* Send a reply */
        sim_logger->info("Generic_fssHardwareModel::command_callback:  Sending reply: %s.", response.c_str());
        _command_node->send_reply_message_async(msg, response.size(), response.c_str());
    }

    /* Custom function to prepare the Generic_fss Data */
    void Generic_fssHardwareModel::create_generic_fss_data(std::vector<uint8_t>& out_data)
    {
        uint8_t four_bytes[4];
        boost::shared_ptr<Generic_fssDataPoint> data_point = boost::dynamic_pointer_cast<Generic_fssDataPoint>(_generic_fss_dp->get_data_point());

        /* Prepare data size */
        out_data.resize(16, 0x00);

        /* Streaming data header - 0xDEADBEEF */
        out_data[0] = 0xDE;
        out_data[1] = 0xAD;
        out_data[2] = 0xBE;
        out_data[3] = 0xEF;

        out_data[4] = 0x01; // command code
        out_data[5] = 0x0A; // length
        
        // alpha
        double_to_4bytes_little_endian(data_point->get_generic_fss_alpha(), four_bytes);
        out_data[6] = four_bytes[0];
        out_data[7] = four_bytes[1];
        out_data[8] = four_bytes[2];
        out_data[9] = four_bytes[3];

        // beta
        double_to_4bytes_little_endian(data_point->get_generic_fss_beta(), four_bytes);
        out_data[10] = four_bytes[0];
        out_data[11] = four_bytes[1];
        out_data[12] = four_bytes[2];
        out_data[13] = four_bytes[3];

        // error code
        out_data[14] = 1; // error
        if (data_point->get_generic_fss_valid()) out_data[14] = 0; // valid

        // checksum
        out_data[15] = compute_checksum(out_data, 4, 11);
    }

    uint8_t Generic_fssHardwareModel::compute_checksum(std::vector<uint8_t>& in, int starting_byte, int number_of_bytes)
    {
        uint32_t sum = 0;
        uint8_t checksum;
        for (int i = starting_byte; i < starting_byte + number_of_bytes; i++) {
            sum += in[i];
        }
        checksum = (uint8_t)(sum & 0x000000FF);
        return checksum;
    }

    void Generic_fssHardwareModel::double_to_4bytes_little_endian(double in, uint8_t out[4])
    {
        union {float f; uint32_t u;} fu;
        fu.f = (float)in;
        uint32_t u = fu.u;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        out[0] = (uint8_t)((u      ) & 0xFF);
        out[1] = (uint8_t)((u >>  8) & 0xFF);
        out[2] = (uint8_t)((u >> 16) & 0xFF);
        out[3] = (uint8_t)((u >> 24) & 0xFF);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        out[0] = (uint8_t)((u >> 24) & 0xFF);
        out[1] = (uint8_t)((u >> 16) & 0xFF);
        out[2] = (uint8_t)((u >>  8) & 0xFF);
        out[3] = (uint8_t)((u      ) & 0xFF);
#else
    #error "__BYTE_ORDER__ is not defined"
#endif
    }

    /* Protocol callback */
    void Generic_fssHardwareModel::uart_read_callback(const uint8_t *buf, size_t len)
    {
        std::vector<uint8_t> out_data; 
        std::uint8_t valid = GENERIC_FSS_SIM_SUCCESS;
        
        std::uint32_t rcv_config;

        /* Retrieve data and log in man readable format */
        std::vector<uint8_t> in_data(buf, buf + len);
        sim_logger->debug("Generic_fssHardwareModel::uart_read_callback:  REQUEST %s",
            SimIHardwareModel::uint8_vector_to_hex_string(in_data).c_str());

        /* Check simulator is enabled */
        if (_enabled != GENERIC_FSS_SIM_SUCCESS)
        {
            sim_logger->debug("Generic_fssHardwareModel::uart_read_callback:  Generic_fss sim disabled!");
            valid = GENERIC_FSS_SIM_ERROR;
        }
        else
        {
            /* Check if message is incorrect size */
            if (in_data.size() != 7)
            {
                sim_logger->debug("Generic_fssHardwareModel::uart_read_callback:  Invalid command size of %d received!", in_data.size());
                valid = GENERIC_FSS_SIM_ERROR;
            }
            else
            {
                /* Check header - 0xDEADBEEF */
                if ((in_data[0] != 0xDE) || (in_data[1] !=0xAD) || (in_data[2] != 0xBE) || (in_data[3] !=0xEF))
                {
                    sim_logger->debug("Generic_fssHardwareModel::uart_read_callback:  Header incorrect!");
                    valid = GENERIC_FSS_SIM_ERROR;
                }
            }

            if (valid == GENERIC_FSS_SIM_SUCCESS)
            {   
                /* Process command */
                switch (in_data[4])
                {
                    case 1:
                        /* Request data */
                        sim_logger->debug("Generic_fssHardwareModel::uart_read_callback:  Send data command received!");
                        create_generic_fss_data(out_data);
                        break;

                    default:
                        /* Unused command code */
                        valid = GENERIC_FSS_SIM_ERROR;
                        sim_logger->debug("Generic_fssHardwareModel::uart_read_callback:  Unused command %d received!", in_data[2]);
                        break;
                }
            }
        }

        /* Increment count and echo command since format valid */
        if (valid == GENERIC_FSS_SIM_SUCCESS)
        {
            _uart_connection->write(&in_data[0], in_data.size());

            /* Send response if existing */
            if (out_data.size() > 0)
            {
                sim_logger->debug("Generic_fssHardwareModel::uart_read_callback:  REPLY %s",
                    SimIHardwareModel::uint8_vector_to_hex_string(out_data).c_str());
                _uart_connection->write(&out_data[0], out_data.size());
            }
        }
    }
}
