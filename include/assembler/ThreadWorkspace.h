#pragma once

#include "finite_element/ElementWorkspace.h"
#include "constitutive/BaseConstitutiveRelation.h"

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
		finite_element::ElementWorkspace element_workspace;
		/// Since the constitutive relations are not thread safe (they have state and temporary
		/// variables), each thread will need its own object.  The elements that this thread owns
		/// will need to be updated to point to this thread's element workspace and constitutive
		/// relation.  This map give a map from the original constitutive relation to the new
		/// one that this thread owns.  It will only be used when the threads are initialized.
		/// However, the map will be kept around until the end of this thread's lifetime since
		/// it actually owns the constitutive relations that the owned elements point to.
		std::map<constitutive::BaseConstitutiveRelation *,
			std::unique_ptr<constitutive::BaseConstitutiveRelation>> constitutive_relations;
	};
}