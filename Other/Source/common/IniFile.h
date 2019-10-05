/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

 /******************************************************************************
  *
  * IniFile : manipulation *.INI
  *
  * Class is a simple wrapper for GetPrivateProfileString
  *
  * *.ini file is cached and not refreshed.
  * IniFile is not thread-aware.
  * IniFile is UTF-8 aware
  *
  * There are two static methods
  *
  * iniRead
  *   directly read one setting from any *.INI
  *
  * iniNames
  *   enumerate all sections or names within section, directly in *.INI
  *
  * IniFile operations:
  *
  * void setDefaults
  *   provide array of iniDefaults structs.
  *     { _T("Section"), _T("Name"),    _T("Default") }, ... }
  * void readIniFile
  *   cache all setings.
  * writeIniFile
  *   put changed settings back to *.INI,
  *   performed in destructor.
  *   writeIniFile changes defaults: current values become default ones.
  * setValue
  *   modify settings
  * getValue
  *   as expected
  * getDefault
  *   as expected
  *   warning: default values are modified using
  *     setDefaults (as expected) and writeIniFile
  * getValueNonEmpty
  *   returns getValue, but if it empty, return default value.
  *   be sure, that defaults are well-defined
  * enumSections *   returns set of known [sections]
  *     namesSet: simple, case insensitive
  * enumNames
  *   returns set of known names within a section
  *     namesSet: simple, case insensitive
  *
  *****************************************************************************/

#include "./common.h"

namespace p_apps {
    class iniFile {

    private:

        class iniValue {
        private:
            std::tstring valueDefault;  // default
            std::tstring valueInterim;  // was read from file
            std::tstring valueCurrent;  // effective
        public:
            iniValue (const std::tstring value, const bool setDefault, const bool setInterim) : valueDefault{}, valueInterim{}, valueCurrent{} {
                setValue (value, setDefault, setInterim);
            }

            void setValue (const std::tstring value, const bool setDefault, const bool setInterim) {
                if (setDefault) {
                    valueDefault = value;
                }
                if (setInterim) {
                    valueInterim = value;
                }
                valueCurrent = value;
            };

            void propagate () {  // update valueInterim to valueCurrent. To be executed after writeIniFile
                valueInterim = valueCurrent;
            };

            std::tstring getDefault () const {
                return valueDefault;
            };

            std::tstring getValue () const {
                return valueCurrent;
            };

            bool isModified () const {
                return !boost::equals (valueInterim, valueCurrent);
            };
        };

        template<typename T>
        struct fuPairLess : std::binary_function < T, T, bool > {
            bool operator() (const std::pair<T, T>& s1, const std::pair<T, T>& s2) const {
                return  boost::ilexicographical_compare (s1.first, s2.first) || (boost::iequals (s1.first, s2.first) && boost::ilexicographical_compare (s1.second, s2.second));
            }
        };

        typedef std::pair<std::tstring, std::tstring> iniKey;

        std::map<iniKey, iniValue, fuPairLess<std::tstring> > values;

        boost::filesystem::path iniName_;
        bool saveOnExit_;

    public:

        struct iniDefaults {
            std::tstring aSection;
            std::tstring aName;
            std::tstring aValue;
        };

        static boost::optional<std::tstring> iniRead (const boost::filesystem::path fName, const std::tstring section, const std::tstring name, const std::tstring defValue = _T ("")) {
            boost::system::error_code errCode;
            if (!boost::filesystem::exists (fName, errCode)) {
                return boost::none;
            }
            std::unique_ptr< TCHAR[] > buf;
            DWORD bufSize = 1024;
            while (true) {
                buf.reset (new (std::nothrow) TCHAR[bufSize]);

                if (buf == nullptr) {
                    break;
                }
                errno = 0;
                DWORD len = GetPrivateProfileString (section.c_str (), name.c_str (), defValue.c_str (), buf.get (), bufSize, fName.c_str ());
                if (ENOENT == errno) {
                    errno = 0;
                    return boost::none;
                }
                if (bufSize - 1 > len) {
                    break;
                }
            #ifdef _WIN64
                const DWORD maxBufSize = UINT_MAX / sizeof buf[0];
            #else
                const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
            #endif
                if (maxBufSize == bufSize) {
                    break;
                }
                bufSize = maxBufSize / 2 >= bufSize ? 2 * bufSize : maxBufSize;
            }
            if (buf == nullptr) {
                return boost::none;
            }
            if (_T ('\0') == buf.get ()) {
                return boost::none;
            }
            return buf.get();
        }

        static void iniNames (const TCHAR *const sectionName, const boost::filesystem::path fName, std::vector<std::tstring>& sections) {
            boost::system::error_code errCode;
            if (!boost::filesystem::exists (fName, errCode)) {
                return;
            }
            std::unique_ptr<TCHAR[]> bufPtr;

            DWORD bufSize = 1024;
            while (true) {
                bufPtr.reset (new (std::nothrow) TCHAR[bufSize]);
                if (!bufPtr) {
                    break;
                }

                DWORD len = GetPrivateProfileString (sectionName, nullptr, nullptr, bufPtr.get (), bufSize, fName.c_str ());
                errno = 0;
                if (ENOENT == errno) {
                    errno = 0;
                    return;
                }
                if (bufSize - 2 > len) {
                    break;
                }

            #ifdef _WIN64
                const DWORD maxBufSize = UINT_MAX / sizeof TCHAR;
            #else
                const DWORD maxBufSize = heapMaxReqReal / sizeof TCHAR;
            #endif
                if (maxBufSize == bufSize) {
                    break;
                }
                bufSize = maxBufSize / 2 >= bufSize ? 2 * bufSize : maxBufSize;
            }
            if (!bufPtr) {
                return;
            }
            TCHAR *ref = bufPtr.get ();
            while (_T ('\0') != *ref) {
                size_t len = wcslen (ref);
                sections.push_back (ref);
                ref += len + 1;
            }
            return;
        }


    public:
        template<typename T>
        struct fuLess : std::binary_function < T, T, bool > {
            bool operator() (const T& s1, const T& s2) const {
                return boost::ilexicographical_compare (s1, s2);
            }
        };

        typedef std::set<std::tstring, fuLess<std::tstring> > namesSet;

    public:
        iniFile () : iniName_ (_T ("")), saveOnExit_ (false) {};
        ~iniFile () {
            writeIniFile ();
        }

        void setDefaults (const std::tstring aSection, const std::tstring aName, const std::tstring aDefault) {
            setValue (aSection, aName, aDefault, true);
        }


        void setDefaults (const iniDefaults *defaults, const size_t nElements = 1) {
            for (size_t idx = 0; nElements > idx; ++idx, ++defaults) {
                setValue (*defaults, true);
            }
        };

        void readIniFile (const boost::filesystem::path iniName, const bool saveOnExit) {
            iniName_ = iniName;
            saveOnExit_ = saveOnExit;
            std::vector<std::tstring> sectionNames;
            iniNames (nullptr, iniName, sectionNames);
            for (auto itSections = begin (sectionNames); end (sectionNames) != itSections; ++itSections) {
                std::vector<std::tstring> nameNames;
                iniNames (itSections->c_str (), iniName, nameNames);
                for (auto itNames = begin (nameNames); end (nameNames) != itNames; ++itNames) {
                    std::tstring defaultValue (_T (""));
                    iniKey thisKey (itSections->c_str (), itNames->c_str ());
                    auto ref = values.find (thisKey);
                    if (values.end () != ref) {
                        defaultValue = ref->second.getDefault ();
                    }
                    boost::optional<std::tstring> valueStr = iniFile::iniRead (iniName, itSections->c_str (), itNames->c_str (), defaultValue);
                    if (valueStr) {
                        if (values.end () == ref) {
                            values.insert (std::pair<iniKey, iniValue> (thisKey, iniValue (valueStr.get (), false, true)));
                        } else {
                            ref->second.setValue (valueStr.get (), false, true);
                        }
                    }
                }
            }
        };

        size_t writeIniFile () {
            if (!saveOnExit_) {
                return 0;
            }
            size_t retVal = 0;
            for (auto it = begin (values); end (values) != it; ++it) {
                if (it->second.isModified ()) {
                    if (WritePrivateProfileString (it->first.first.c_str (), it->first.second.c_str (), !it->second.getValue ().empty () ? it->second.getValue ().c_str () : nullptr, iniName_.c_str ())) {
                        it->second.propagate ();
                        ++retVal;
                    }
                }
            }
            return retVal;
        }

        boost::optional<std::tstring> getValue (const std::tstring section, const std::tstring name) const {
            iniKey thisKey (section, name);
            auto ref = values.find (thisKey);
            if (values.end () == ref) {
                return boost::none;
            }
            std::tstring retVal = ref->second.getValue ();
            if (retVal.empty ()) {
                return boost::none;
            }
            return retVal;
        }

        boost::optional<std::tstring> getDefault (const std::tstring section, const std::tstring name) const {
            iniKey thisKey (section, name);
            auto ref = values.find (thisKey);
            if (values.end () == ref) {
                return boost::none;
            }
            std::tstring retVal = ref->second.getDefault ();
            if (retVal.empty ()) {
                return boost::none;
            }
            return retVal;
        }

        std::tstring getValueNonEmpty (const std::tstring section, const std::tstring name) const {
            iniKey thisKey (section, name);
            auto ref = values.find (thisKey);
            if (values.end () == ref) {
                return _T ("");
            }
            std::tstring retVal = ref->second.getValue ();
            if (retVal.empty ()) {
                retVal = ref->second.getDefault ();
            }
            return retVal;
        }

        void enumSections (namesSet& result) const {
            for (auto it = begin (values); end (values) != it; ++it) {
                result.insert (it->first.first);
            }
        }

        void enumNames (const std::tstring section, namesSet& result) const {
            for (auto it = begin (values); end (values) != it; ++it) {
                if (boost::iequals (section, it->first.first)) {
                    result.insert (it->first.second);
                }
            }
        }

        void setValue (const std::tstring section, const std::tstring name, const std::tstring value, const bool isDefault = false) {
            iniKey thisKey (section, name);
            auto ref = values.find (thisKey);
            if (values.end () == ref) {
                values.insert (std::pair<iniKey, iniValue> (thisKey, iniValue (value, isDefault, false)));
            } else {
                ref->second.setValue (value, isDefault, false);
            }
        }

        void setValue (const iniDefaults& defaults, const bool isDefault = false) {
            setValue (defaults.aSection, defaults.aName, defaults.aValue, isDefault);
        }

    };
}
