#include "assembler/SerialAssembler.h"

#include "utility/Exceptions.h"
#include "finite_element/FiniteElement.h"
#include "finite_element/BaseFormulation.h"

#include <memory>

namespace assembler
{
	void assembler::SerialAssembler::execute(const sparse_linear_algebra::Vector & gl_solution_vector)
	{
		// Set any global matrices / vectors to zero that require assembly
		// so the element terms can be added
		for (auto& matrix_tuple : this->queued_matrix_assemblies_)
		{
			std::get<0>(matrix_tuple)->start_assembly();
		}
		for (auto& vector_tuple : this->queued_vector_assemblies_)
		{
			std::get<0>(vector_tuple)->start_assembly();
		}

		throw _UTIL_MSG_EXCEPT_C("TODO: Finish implementation of SerialAssembler::execute");
		// Now execute the element and assemblies operations
		simple_math::GeneralVector elem_solution_vector;
		for (auto& e_ptr : this->mesh_.elements())
		{
			// Provide the current solution in case the element requries it.
			elem_solution_vector = gl_solution_vector.get_element_part(e_ptr->get_local_number());

			// Start calculation block
			e_ptr->start_calculations(elem_solution_vector);

			// Execute queued element operations
			for (ElementOperation& operation : queued_operations_)
			{
				operation(*e_ptr);
			}

			// Execute the queued matrix assemblies
			for (auto& matrix_tuple : queued_matrix_assemblies_)
			{
				auto gl_matrix = std::get<0>(matrix_tuple);
				
				// For the case that the global matrix is "matrix-free" in that the global
				// system is never assembled, provide the element matrices for elements
				// that have had an internal state variable change or have kinematic nonlinearities
				// combined with a change in the global solution vector.
				//if (!gl_matrix.supports_local_updates()) || e_ptr->requires_update())
				//{
					gl_matrix->add_element_matrix(e_ptr->get_local_number(),
						std::get<1>(matrix_tuple)(*e_ptr));
				//}
			}

			// Execute the queued vector assemblies
			// This assumes global vectors will always be assembled
			for (auto& vector_tuple : queued_vector_assemblies_)
			{
				auto gl_vector = std::get<0>(vector_tuple);

				gl_vector->add_element_vector(e_ptr->get_local_number(),
					std::get<1>(vector_tuple)(*e_ptr));
			}
			
			// End calculation block
			e_ptr->end_calculations();
		}


		// Indicate to the aseembled matrices/vectors that assembly is done
		for (auto& matrix_tuple : this->queued_matrix_assemblies_)
		{
			std::get<0>(matrix_tuple)->end_assembly();
		}
		for (auto& vector_tuple : this->queued_vector_assemblies_)
		{
			std::get<0>(vector_tuple)->end_assembly();
		}

		queued_operations_.clear();
		queued_matrix_assemblies_.clear();
		queued_vector_assemblies_.clear();
	}
}