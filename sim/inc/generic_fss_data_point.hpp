#ifndef NOS3_GENERIC_FSSDATAPOINT_HPP
#define NOS3_GENERIC_FSSDATAPOINT_HPP

#include <boost/shared_ptr.hpp>
#include <sim_42data_point.hpp>

namespace Nos3
{
    /* Standard for a data point used transfer data between a data provider and a hardware model */
    class Generic_fssDataPoint : public SimIDataPoint
    {
    public:
        /* Constructors */
        Generic_fssDataPoint(double count);
        Generic_fssDataPoint(int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        /* Accessors */
        /* Provide the hardware model a way to get the specific data out of the data point */
        std::string to_string(void) const;
        double      get_generic_fss_data_x(void) const {return _generic_fss_data[0];}
        double      get_generic_fss_data_y(void) const {return _generic_fss_data[1];}
        double      get_generic_fss_data_z(void) const {return _generic_fss_data[2];}
        bool        is_generic_fss_data_valid(void) const {return _generic_fss_data_is_valid;}
    
    private:
        /* Disallow these */
        Generic_fssDataPoint(void) {};
        Generic_fssDataPoint(const Generic_fssDataPoint&) {};
        ~Generic_fssDataPoint(void) {};

        /* Specific data you need to get from the data provider to the hardware model */
        /* You only get to this data through the accessors above */
        mutable bool   _generic_fss_data_is_valid;
        mutable double _generic_fss_data[3];
    };
}

#endif
