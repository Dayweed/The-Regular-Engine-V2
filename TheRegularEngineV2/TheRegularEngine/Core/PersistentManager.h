#pragma once
/*!
	@file		PersistentManager.h
	@author		Isaiah Lim (Code Contribution 100%)
	@email		lim.i@digipen.edu
	@date		08/09/2023
	@brief		Handles "Persistent" Values across scenes, will not be
				serialized so it will be cleared once the game/engine close

	Copyright (C) 2023 DigiPen Institute of Technology.
	Reproduction or disclosure of this file or its contents without the
	prior written consent of DigiPen Institute of Technology is prohibited.
************************************************************************/

namespace TRE
{
	class PersistentManager
	{
		friend class ECSManager;

	public:
		static PersistentManager& Instance()
		{
			static PersistentManager instance;
			return instance;
		}

		/* !
		@function		GetVariableValue
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Get variable in m_Variables

		[Warning]		Will return an empty string if it is a new variable
		*//*__________________________________________________________________________*/
		std::string GetVariableValue(std::string variableName);

		/* !
		@function		SetVariableValue
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Set variable in m_Variables
		*//*__________________________________________________________________________*/
		void SetVariableValue(std::string variableName, std::string variableValue);

		/* !
		@function		SetVariable
		@author			Isaiah Lim (lim.i@digipen.edu)

		@brief			Get variable in m_Variables

		[Warning]		Clears all variables inside it
		*//*__________________________________________________________________________*/
		void ClearVariables();

	private:
		friend class GameLoop;

		// Delete possible copy ctor and assignment to ensure singleton
		PersistentManager() {};
		PersistentManager(PersistentManager const&) = delete;
		void operator=(PersistentManager const&) = delete;
		void* operator new(size_t) = delete;

		std::map<std::string, std::string> m_Variables;
	};
}