/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#pragma once

/******************************************************************************
 *
 * There a static method
 *
 * expandEnv
 *   Expand environment variable strings
 *
 * class pApps::Environment represents set of environment variables
 * Environment is a simple, case insensitive map of tstrings
 * Environment should be initialized from envp[] array
 * manipulated using get/set/erase
 * then dumped to another envp[] array and passed to launched program
 *
 * Probably there will exist a single object of class Environment
 *
 *****************************************************************************/

#include "./common.h"

namespace pApps {
	class Environment {
		private:
			template<typename T>
			struct fuLess : std::binary_function < T, T, bool > {
				bool operator() ( const T& s1, const T& s2 ) const {
					return boost::ilexicographical_compare ( s1, s2 );
				}
			};
			typedef std::map<std::tstring, std::tstring, fuLess<std::tstring>> IMap;

			IMap _mEnv;

		public:
//			Environment() = delete;
//			Environment( const Environment& ) = delete;
//			Environment( const Environment&& ) = delete;
//			Environment& Environment:: operator= ( const Environment& ) = delete;
//			Environment&& Environment:: operator= ( const Environment&& ) = delete;

			Environment() { };

			Environment ( const TCHAR *const envp[ ] ) : Environment() {
				setUp ( envp );
			}

			typedef DWORD( WINAPI *ExpandEnvironmentStrings_t ) ( LPCTSTR, LPWSTR, DWORD );

			static std::tstring expandEnv( const std::tstring& str, ExpandEnvironmentStrings_t ExpandEnvironmentStrings_f = ExpandEnvironmentStrings ) {
				auto bufSize = ExpandEnvironmentStrings_f ( str.c_str(), nullptr, 0 );

				std::unique_ptr< TCHAR[ ] > buf ( new ( std::nothrow ) TCHAR[ bufSize ] );
				if ( nullptr == buf ) {
					return str;
				}
				if ( bufSize != ExpandEnvironmentStrings_f ( str.c_str(), buf.get(), bufSize ) ) {
					return str;
				}
				return buf.get();
			}

			void setUp ( const TCHAR *const envp[ ] ) {
				if ( nullptr == envp ) {
					return;
				}
				for ( auto *ref = envp; nullptr != *ref; ++ref ) {
					auto *sep = _tcschr ( *ref, _T ( '=' ) );
					if ( nullptr == sep ) {
						continue;
					}
					if ( sep == *ref ) {
						continue;
					}
					size_t len = sep - *ref;
					sep++;
					if ( _T ( '\0' ) == *sep ) {
						continue;
					}
					std::tstring name ( *ref, len );
					_mEnv.insert ( std::pair<std::tstring, std::tstring> ( name, sep ) );
				}
			}

			bool exists ( const std::tstring name ) const {
				if ( 0 == name.length() ) {
					return false;
				}
				return _mEnv.end() != _mEnv.find ( name );
			}

			std::tstring get ( const std::tstring name ) {
				if ( 0 == name.length() ) {
					return _T ( "" );
				}
				return _mEnv[ name ];
			}

			void set ( const std::tstring name, const std::tstring value, const std::tstring keyBackup = _T ( "" ) ) {
				if ( keyBackup.length() != 0 ) {
					_mEnv[ keyBackup ] = _mEnv[ name ];
				}
				_mEnv[ name ] = value;
			}

			void erase ( const std::tstring name ) {
				_mEnv.erase ( name );
			}

			void dump ( std::vector<std::tstring>& result ) const {
				for ( const auto& it : _mEnv ) {
					std::tstring val = it.first + _T ( "=" ) + it.second;
					result.push_back ( val );
				}
			}

			void dump ( std::unique_ptr<TCHAR[ ]>& result ) const {
				size_t len = 2 * _mEnv.size() + 1; // equal signs, zeros after every string, extra zero
				for ( const auto& it : _mEnv ) {
					len += it.first.length() + it.second.length();
				}
				result.reset ( new ( std::nothrow ) TCHAR[ len ] );

				if ( !result ) {
					return;
				}

				TCHAR *ref = result.get();

				for ( const auto& it : _mEnv ) {
					TCHAR *xyz = ref;
					size_t thisLen = it.first.length();
					wcsncpy_s ( ref, len, it.first.c_str(), thisLen );
					ref += thisLen;
					len -= thisLen;
					* ( ref++ ) = _T ( '=' );
					--len;
					thisLen = it.second.length();
					wcsncpy_s ( ref, len, it.second.c_str(), thisLen );
					ref += thisLen;
					len -= thisLen;
					* ( ref++ ) = _T ( '\0' );
					--len;
				}
				* ( ref++ ) = _T ( '\0' );
				--len;
				assert ( !len );
			}
	};
}
