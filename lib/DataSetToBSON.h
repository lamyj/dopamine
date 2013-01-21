#ifndef _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09
#define _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09

#include <gdcmDataElement.h>
#include <gdcmDataSet.h>
#include <gdcmVR.h>

#include <iconv.h>

#include <mongo/bson/bson.h>

/**
 * @brief Convert a GDCM DataSet to a BSON object.
 */
class DataSetToBSON
{
public :

    /*
    // cf. https://httpd.apache.org/docs/2.2/en/mod/mod_authz_host.html#order
    struct Order
    {
        enum Type
        {
            INCLUDE_FIRST=0,
            EXCLUDE_FIRST,
            MAX
        };
    };
    */

    DataSetToBSON();
    ~DataSetToBSON();

    std::string get_specific_character_set() const;
    void set_specific_character_set(std::string const & specific_character_set);

    //Order::Type const & get_order() const;
    //void set_order(Order::Type const & order);
    // TODO : include and exclude filters (don't forget "ALL" rule)

    void operator()(gdcm::DataSet const & dataset, mongo::BSONObjBuilder & builder);

private :
    std::string _specific_character_set;
    iconv_t _converter;

    //Order::Type _order;

    void _add_element(gdcm::DataElement const & element,
                      mongo::BSONObjBuilder & builder) const;
    void _add_element(char const * begin, char const * end, gdcm::VR const & vr,
                      mongo::BSONArrayBuilder & builder) const;
    unsigned long _get_length(char const * begin, char const * end,
                              gdcm::VR const & vr) const;

};

#endif // _737cc322_0e2e_4fbb_aac6_b7df5e4f2d09
