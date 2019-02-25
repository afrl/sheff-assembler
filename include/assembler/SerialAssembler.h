#pragma once

#include "Assembler.h"

namespace assembler
{
	class SerialAssembler : public Assembler
	{
	public:
		/// Execute any queued operations/assemblies.  This will clear the queued operations/assemblies
		/// once complete.  This provides the serial implementation of execute.
		virtual void execute(const sparse_linear_algebra::Vector& gl_solution_vector) override;
	};
}