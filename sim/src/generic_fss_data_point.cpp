#include <ItcLogger/Logger.hpp>
#include <generic_fss_data_point.hpp>

namespace Nos3
{
    extern ItcLogger::Logger *sim_logger;

    Generic_fssDataPoint::Generic_fssDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp)
    {
        sim_logger->trace("Generic_fssDataPoint::Generic_fssDataPoint:  42 Constructor executed");

        bool parsedValid = false;
        bool parsedSunang = false;
        /* Initialize data */
        _generic_fss_valid = false;
        _generic_fss_alpha = 0.0;
        _generic_fss_beta = 0.0;

        /*
        ** Declare 42 telemetry string prefix
        ** 42 variables defined in `42/Include/42types.h`
        ** 42 data stream defined in `42/Source/IPC/SimWriteToSocket.c`
        */
        std::ostringstream MatchStringStream;
        MatchStringStream << "SC[" << spacecraft << "].AC.FSS[0]"; /* TODO: Change me to match the data from 42 you are interested in */
        std::string MatchString = MatchStringStream.str();
        size_t MSsize = MatchString.size();

        /* Parse 42 telemetry */
        std::vector<std::string> lines = dp->get_lines();
        try 
        {
            for (unsigned int i = 0; i < lines.size(); i++) 
            {
                /* Compare prefix */
                if (lines[i].compare(0, MSsize, MatchString) == 0) 
                {
                    size_t found = lines[i].find_first_of("=");
                    /* Parse line */
                    std::istringstream iss(lines[i].substr(found+1, lines[i].size()-found-1));
                    std::string s;
                    /* Custom work to extract the data from the 42 string and save it off in the member data of this data point */
                    if (lines[i].compare(MSsize+1, 5, "Valid") == 0) {
                        _generic_fss_valid = false;
                        iss >> s;
                        if (s.compare(0, 1, "1") == 0) _generic_fss_valid = true;
                        parsedValid = true;
                    } else if (lines[i].compare(MSsize+1, 6, "SunAng") == 0) {
                        iss >> s;
                        _generic_fss_alpha = std::stod(s);
                        iss >> s;
                        _generic_fss_beta = std::stod(s);
                        parsedSunang = true;
                    }
                    /* Debug print */
                }
            }
            if (parsedValid && parsedSunang) {
                sim_logger->trace("Generic_fssDataPoint::Generic_fssDataPoint:  Parsed valid = %s, sunang = %f %f", _generic_fss_valid?"True":"False", _generic_fss_alpha, _generic_fss_beta);
            }
        } 
        catch(const std::exception& e) 
        {
            /* Report error */
            sim_logger->error("Generic_fssDataPoint::Generic_fssDataPoint:  Parsing exception %s", e.what());
        }
    }

    /* Used for printing a representation of the data point */
    std::string Generic_fssDataPoint::to_string(void) const
    {
        sim_logger->trace("Generic_fssDataPoint::to_string:  Executed");
        
        std::stringstream ss;

        ss << std::fixed << std::setfill(' ');
        ss << "Generic_fss Data Point:   Valid: ";
        ss << std::setprecision(std::numeric_limits<double>::digits10); /* Full double precision */
        ss << " Generic_fss valid: "
           << (_generic_fss_valid?"True":"False")
           << "alpha: "
           << _generic_fss_alpha
           << "beta: "
           << _generic_fss_beta;

        return ss.str();
    }
} /* namespace Nos3 */
