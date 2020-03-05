#pragma once

#include "finite_element/workspaces/ElementWorkspace.h"

#include <map>
#include <memory>

namespace assembler
{
	/// \brief A workspace for a thread to perform element calculations.
	///
	/// \attention It should be noted that the design of the algorithms is only thread-safe
	/// when each thread has its own workspace.  Any two AssemblerThread objects should never
	/// have the same workspace.
	struct ThreadWorkspace
	{
		/// The workspace for elements to use while performing calculations.
		std::unique_ptr<finite_element::ElementWorkspace> element_workspace_ptr;

        ThreadWorkspace()
        {
            element_workspace_ptr = std::make_unique<finite_element::ElementWorkspace>();
        }
	};
}