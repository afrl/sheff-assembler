#pragma once

#include "Assembler.h"
#include "ThreadWorkspace.h"

namespace assembler
{
	class SerialAssembler : public Assembler
	{
	public:
		using Assembler::Assembler;
		virtual ~SerialAssembler() {}

		/// Execute any queued operations/assemblies.  This will clear the queued operations/assemblies
		/// once complete.  This provides the serial implementation of execute.
        virtual void execute(const std::vector<double>& local_solution_vector,
                             const dof_manager::DofMap& dof_map) override;

	private:
		/// For a serial assembler, this is only one thread, so the assembler holds the workspace
		ThreadWorkspace workspace_;
	};
}