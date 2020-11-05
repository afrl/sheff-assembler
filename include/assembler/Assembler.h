#pragma once

#include "finite_element/FiniteElementMesh.h"
#include "sparse_linear_algebra/SparseMatrix.h"
#include "sparse_linear_algebra/Vector.h"

#include <deque>
#include <functional>

namespace assembler
{
	/// A class that provides the implementation of assembling element matrices/vectors into
	/// global matrices/vectors.  It also provides a way to calling a function on all of the
	/// elements.  There will be serial and parallel implementations of this.  For distributed
	/// analyses with 1 rank per a core, a serial assembler should be used.  For analyses with
	/// multiple cores per a rank (or only one rank), the parallel assembler will aceelerate
	/// assembly and other element loops.
	class Assembler
	{
	public:
		using ElementOperation = std::function<void(finite_element::FiniteElementAccessor& fe)>;
		using ElementMatrixReturnType = simple_math::EigenBlockMatrix;
		using ElementMatrixCallback = std::function<ElementMatrixReturnType(finite_element::FiniteElementAccessor& fe)>;
		using ElementVectorReturnType = simple_math::EigenBlockVector;
		using ElementVectorCallback = std::function<ElementVectorReturnType(finite_element::FiniteElementAccessor& fe)>;

		/// @brief Constructor
		/// @param fe_mesh - finite element mesh that contains the topological mesh and element
		///        layouts.  Note: an assembler object will hold a persistent reference to the
		///        given finite_element::FiniteElementMesh object for the assembler's lifetime.
		///        Additionally, the mesh object is non-const because the field/enrichment
		///        data within the layouts needs to be modifiable for opperations like updating
		///        ISV's in a field/enrichment.
		Assembler(finite_element::FiniteElementMesh& fe_mesh);
		virtual ~Assembler() {}

		/// Queue an operation on the element.  This could be updating internal state variables,
		/// calculating information output, etc.
		void queue_operation(const ElementOperation& operation);

		/// Queue a element to global matrix assembly operation.
        /// \param flag - indicates what quantity at the element level to calculate, which will be
        ///        assembled into the global matrix.
        /// \param gl_matrix - the global matrix to assemble the element quantities into.
		void queue_assembly(
            const finite_element::FEMatricesAndVectors::Flags flag,
            sparse_linear_algebra::SparseMatrix * gl_matrix);

        /// Queue a element to global vector assembly operation.
        /// \param flag - indicates what quantity at the element level to calculate, which will be
        ///        assembled into the global vector.
        /// \param gl_matrix - the global vector to assemble the element quantities into.
        void queue_assembly(
            const finite_element::FEMatricesAndVectors::Flags flag,
            sparse_linear_algebra::Vector* gl_vector);

		/// Execute any queued operations/assemblies.  This will clear the queued
		/// operations/assemblies once complete.
		virtual void execute(const sparse_linear_algebra::Vector * const gl_solution_vector) = 0;

	protected:
        using MatrixAssemblyTuple = std::tuple<sparse_linear_algebra::SparseMatrix*, const finite_element::FEMatricesAndVectors::Flags>;
        using VectorAssemblyTuple = std::tuple<sparse_linear_algebra::Vector*, const finite_element::FEMatricesAndVectors::Flags>;

		std::deque<ElementOperation> queued_operations_;
		std::deque<MatrixAssemblyTuple> queued_matrix_assemblies_;
		std::deque<VectorAssemblyTuple> queued_vector_assemblies_;

		/// A const reference to mesh that this assembler is responsible for
        finite_element::FiniteElementMesh& mesh_;
	};

	/////////////////
	// Definitions //
	/////////////////

	inline assembler::Assembler::Assembler(finite_element::FiniteElementMesh& mesh)
		: mesh_(mesh)
	{
	}

	inline void assembler::Assembler::queue_operation(const ElementOperation & operation)
	{
		queued_operations_.push_back(operation);
	}

    inline void Assembler::queue_assembly(
        const finite_element::FEMatricesAndVectors::Flags flag, 
        sparse_linear_algebra::SparseMatrix* gl_matrix)
    {
        finite_element::FEMatricesAndVectors::Bitset flags_allowed =
            utility::mask(finite_element::FEMatricesAndVectors::Flags::LHSMatrix);

        _UTIL_ASSERT_C(flags_allowed.contains(flag),
            "The given flag is not allowed to be assembled into a globla matrix.  "
            "Either the flag is for a vector quantity or a new flag was added to "
            "the code and this method needs to be updated.");

        queued_matrix_assemblies_.push_back(MatrixAssemblyTuple(gl_matrix, flag));
    }

    inline void Assembler::queue_assembly(
        const finite_element::FEMatricesAndVectors::Flags flag, 
        sparse_linear_algebra::Vector* gl_vector)
    {
        finite_element::FEMatricesAndVectors::Bitset flags_allowed =
            utility::mask(finite_element::FEMatricesAndVectors::Flags::RHSVector) |
            utility::mask(finite_element::FEMatricesAndVectors::Flags::CompleteRHSVector);

        _UTIL_ASSERT_C(flags_allowed.contains(flag),
            "The given flag is not allowed to be assembled into a globla matrix.  "
            "Either the flag is for a vector quantity or a new flag was added to "
            "the code and this method needs to be updated.");

        queued_vector_assemblies_.push_back(VectorAssemblyTuple(gl_vector, flag));
    }
}