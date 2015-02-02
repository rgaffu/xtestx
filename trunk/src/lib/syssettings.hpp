/**
******************************************************************************
* @file    syssetting.hpp
* @brief   A collection of useful operating system related routines
*
* @author  
* @version V1.0.0
* @date    30-Jan-2015
*          
* @verbatim
* @endverbatim
*
******************************************************************************
* @attention
*
******************************************************************************
* @note
*
*****************************************************************************/

/*Include only once */
#ifndef __SYSSETTINGS_HPP_INCLUDED
#define __SYSSETTINGS_HPP_INCLUDED

#ifndef __cplusplus
#error syssetting.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////
#include <linux/types.h>

/////////////////////////////////////////////////////////////////////////////
/// Classe: system settings.
///
/// \dot	digraph {
///		}
///	\enddot
class SysSettings
{
	
//--- Funzioni ---
public:
	SysSettings();
	~SysSettings();

	/// Scrittura data e ora.
	///
	/// \param[in]	day = giorno
	/// \param[in]	month = mese
	/// \param[in]	year = anno
	///
	/// \return		none
	void systemDateTime_set(char day, char month, int year, char hour, char minute, char second, bool hwSet);

	/// Richiesta versione linux kernel.
	///
	/// \param[in]	none 
	///
	/// \return		versione linux kernel
	char *systemUname_request(void);

	/// Richiesta reboot del sistema.
	///
	/// \param[in]	none 
	///
	/// \return		none
	void systemReboot_request(void);
	
protected:
	
private:

//--- Variabili ---
};

/****************************************************************************/

#endif /* __SYSSETTINGS_HPP_INCLUDED */
/* EOF */

