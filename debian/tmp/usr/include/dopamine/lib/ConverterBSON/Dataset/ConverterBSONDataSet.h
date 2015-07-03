/*************************************************************************
 * dopamine - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef _36bbbced_f7c3_45e7_aa64_89253348c949
#define _36bbbced_f7c3_45e7_aa64_89253348c949

#include <string>
#include <vector>

namespace dopamine
{

namespace converterBSON
{

/**
 * @brief \class Base class to Convert Bson to Dataset or
 * Dataset to Bson
 */
class ConverterBSONDataSet
{
public:
    /// Create an instance of ConverterBSONDataSet
    ConverterBSONDataSet(bool isDcmToBSON);
    
    /// Destroy the instance of ConverterBSONDataSet
    virtual ~ConverterBSONDataSet();

    /**
     * Get Specific character set
     * @return Specific character set
     */
    std::string get_specific_character_set() const;
    
    /**
     * Set Specific character set
     * @param specific_character_set: new character set
     */
    virtual void set_specific_character_set(
            std::string const & specific_character_set);

protected:
    /// Character Set
    std::vector<std::string> _specific_character_sets;

private:
    /// Flag sens of convertion
    bool _isDcmToBSON;

};

} // namespace converterBSON

} // namespace dopamine

#endif // _36bbbced_f7c3_45e7_aa64_89253348c949
