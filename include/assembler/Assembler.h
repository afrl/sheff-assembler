#pragma once

#include "finite_element/FiniteElement.h"
#include "sparse_linear_algebra/SparseMatrix.h"
#include "sparse_linear_algebra/Vector.h"
#include "flex_mesh/mesh/Mesh.h"

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
		using ElementOperation = std::function<void(finite_element::FiniteElement& fe)>;
		using ElementMatrixReturnType = simple_math::EigenBlockMatrix;
		using ElementMatrixCallback = std::function<ElementMatrixReturnType(finite_element::FiniteElement& fe)>;
		using ElementVectorReturnType = simple_math::EigenBlockVector;
		using ElementVectorCallback = std::function<ElementVectorReturnType(finite_element::FiniteElement& fe)>;

		Assembler(const flex_mesh::Mesh<flex_mesh::Node, finite_element::FiniteElement>& mesh);
		virtual ~Assembler() {}

		/// Queue an operation on the element.  This could be updating internal state variables, calculating
		/// information output, etc.
		void queue_operation(const ElementOperation& operation);
		/// Queue a element to global matrix assembly operation.
		void queue_matrix_assembly(sparse_linear_algebra::SparseMatrix * gl_matrix,
			const ElementMatrixCallback& get_element_matrix_callback);
		/// Queue a element to global vector assembly operation.
		void queue_vector_assembly(sparse_linear_algebra::Vector * gl_vector,
			const ElementVectorCallback& get_element_vector_callback);
		/// Execute any queued operations/assemblies.  This will clear the queued operations/assemblies
		/// once complete.
		virtual void execute(const sparse_linear_algebra::Vector& gl_solution_vector) = 0;

	protected:
		std::deque<ElementOperation> queued_operations_;
		std::deque<std::tuple<sparse_linear_algebra::SparseMatrix *, const ElementMatrixCallback&>> queued_matrix_assemblies_;
		std::deque<std::tuple<sparse_linear_algebra::Vector *, const ElementVectorCallback&>> queued_vector_assemblies_;

		/// A const reference to mesh that this assembler is responsible for
		const flex_mesh::Mesh<flex_mesh::Node, finite_element::FiniteElement>& mesh_;
	};

	/////////////////
	// Definitions //
	/////////////////

	inline assembler::Assembler::Assembler(const flex_mesh::Mesh<flex_mesh::Node, finite_element::FiniteElement>& mesh)
		: mesh_(mesh)
	{
	}

	inline void assembler::Assembler::queue_operation(const ElementOperation & operation)
	{
		queued_operations_.push_back(operation);
	}

	inline void Assembler::queue_matrix_assembly(sparse_linear_algebra::SparseMatrix * gl_matrix, const ElementMatrixCallback & get_element_matrix_callback)
	{
		queued_matrix_assemblies_.push_back(std::tie<sparse_linear_algebra::SparseMatrix *, const ElementMatrixCallback&>(gl_matrix, get_element_matrix_callback));
	}

	inline void Assembler::queue_vector_assembly(sparse_linear_algebra::Vector * gl_vector, const ElementVectorCallback & get_element_vector_callback)
	{
		queued_vector_assemblies_.push_back(std::tie<sparse_linear_algebra::Vector *, const ElementVectorCallback&>(gl_vector, get_element_vector_callback));
	}
}