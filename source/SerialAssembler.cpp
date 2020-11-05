#include "assembler/SerialAssembler.h"

#include "finite_element/FiniteElement.h"
#include "utility/Exceptions.h"
#include "utility/MPIUtils.h"

#include <memory>

namespace assembler
{
void assembler::SerialAssembler::execute(const sparse_linear_algebra::Vector* gl_solution_vector)
{
    // Set any global matrices / vectors to zero that require assembly
    // so the element terms can be added
    for (auto& matrix_tuple : this->queued_matrix_assemblies_)
    {
        std::get<0>(matrix_tuple)
          ->start_assembly(sparse_linear_algebra::ElementAssemblyType::INSERT_ALL);
    }
    for (auto& vector_tuple : this->queued_vector_assemblies_)
    {
        std::get<0>(vector_tuple)->start_assembly();
    }

    // Now execute the element and assemblies operations
    simple_math::GeneralVector elem_solution_vector;
    for (auto& fe : mesh_.finite_elements())
    {
        // Provide the current solution in case the element requries it.
        elem_solution_vector = gl_solution_vector->get_element_part(fe.element.local_index());

        auto params = finite_element::FiniteElement::make_params(fe.element,
                                                                 elem_solution_vector,
                                                                 fe.layout,
                                                                 *workspace_.element_workspace_ptr);

        // Execute queued element operations
        for (ElementOperation& operation : queued_operations_)
        {
            operation(fe);
        }

        if (fe.element.parent_partition() == utility::MPIUtils::get_local_rank())
        {
            // Build a bitset of all of the flags for the element assembly
            finite_element::FEMatricesAndVectors::Bitset all_set_flags;
            for (auto& matrix_tuple : queued_matrix_assemblies_)
            {
                all_set_flags.flip(std::get<1>(matrix_tuple));
            }
            for (auto& vector_tuple : queued_vector_assemblies_)
            {
                all_set_flags.flip(std::get<1>(vector_tuple));
            }

            auto&& result = finite_element::FiniteElement::calculate(params, all_set_flags);

            // Now assemble the results into global matrices/vectors
            for (auto& matrix_tuple : queued_matrix_assemblies_)
            {
                auto gl_matrix        = std::get<0>(matrix_tuple);
                auto&& element_matrix = result.get_matrix(std::get<1>(matrix_tuple));

                // For the case that the global matrix is "matrix-free" in that the global
                // system is never assembled, provide the element matrices for elements
                // that have had an internal state variable change or have kinematic nonlinearities
                // combined with a change in the global solution vector.
                // if (!gl_matrix.supports_local_updates()) || e_ptr->requires_update())
                //{
                gl_matrix->add_element_matrix(fe.element.local_index(), std::move(element_matrix));
                //}
            }
            for (auto& vector_tuple : queued_vector_assemblies_)
            {
                auto gl_vector        = std::get<0>(vector_tuple);
                auto&& element_vector = result.get_vector(std::get<1>(vector_tuple));

                gl_vector->add_element_vector(fe.element.local_index(), std::move(element_vector));
            }
        }
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