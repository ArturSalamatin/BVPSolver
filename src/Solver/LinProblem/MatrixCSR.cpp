#include <fstream>
#include <mutex>
#include "MatrixCSR.h"
#include "SparsityPattern.h"


	namespace linear_problem
	{
		const SparsityPattern& MatrixCSR::sparsity_pattern() const
		{
			return *pattern.get();
		}

		unsigned char MatrixCSR::EqNmbr() const { return sparsity_pattern().EqNmbr(); }

		/////////////////// MatrixCSR
		void MatrixCSR::CopyBlock(size_t valueOffset, std::vector<double>& dest, const std::vector<double>& data, const std::vector<size_t>& blockPosInValArray)
		{
			static std::mutex CopyBlockMutex;
			std::lock_guard<std::mutex> my_lock(CopyBlockMutex);
			for (size_t i = 0; i < NmbrOfNonZerosPerUnitBlock(); i++)
				dest[blockPosInValArray[valueOffset + i]] += data[i];
		}

		size_t MatrixCSR::NmbrOfNonZerosPerUnitBlock() const { return sparsity_pattern().NmbrOfNonzerosPerUnitBlock(); }

		const std::vector<size_t>& MatrixCSR::Row() const { return sparsity_pattern().Row(); }

		const std::vector<size_t>& MatrixCSR::Col() const { return sparsity_pattern().Col(); }

		const std::vector<double>& MatrixCSR::Val() const { return value; }

		void MatrixCSR::AddDiagBlock(size_t l, const std::vector<double>& data)
		{
			// diagonal elements are stored separately, block after block
			size_t valueOffset = l * NmbrOfNonZerosPerUnitBlock();
			CopyBlock(valueOffset, value, data, sparsity_pattern().DiagBlocks());
		}

		void MatrixCSR::AddOffDiagBlock(size_t l, size_t neibIdx, std::vector<double>& data)
		{
			// in (neibIdx - l)
			// - l subtracts the diagonal blocks for the valueOffset. The diagonal blocks are stored separately
			// +neibIdx puts the pointer to the correct neighbour first element
			size_t valueOffset = sparsity_pattern().NmbrOfElementsAboveBlockRow()[l] + (neibIdx - l) * NmbrOfNonZerosPerUnitBlock();
			CopyBlock(valueOffset, value, data, sparsity_pattern().OffDiagBlocks());
		}

		MatrixCSR::MatrixCSR() noexcept = default;

		MatrixCSR::MatrixCSR(
			const size_t eqNmbr_, const size_t cellNmbr, 
			const std::vector<std::vector<int>>&connectivityGraph)  noexcept :
			MatrixCSR(eqNmbr_, cellNmbr,
				connectivityGraph, std::vector<bool>(true, eqNmbr_ * eqNmbr_))
		{}

		MatrixCSR::MatrixCSR(
			size_t eqNmbr_, size_t cellNmbr,
			const std::vector<std::vector<int>>&connectivityGraph,
			const std::vector<bool>&blPattern)  noexcept :
			pattern{ std::make_unique<SparsityPattern>(eqNmbr_, cellNmbr, connectivityGraph, blPattern) },
			nnz{ sparsity_pattern().TotalNmbrOfBlocks() * sparsity_pattern().NmbrOfNonzerosPerUnitBlock() }
		{
			ResetMatrix();
		}

		MatrixCSR::~MatrixCSR() = default;

		void MatrixCSR::ResetMatrix()
		{
			value = std::vector<double>(nnz, 0.0);
		}

		void MatrixCSR::PrintCRS() const
		{
			std::ofstream myfile;
			myfile.open("test_Matrix.txt");
			sendCRS2Stream(myfile);
			myfile.close();
		}

		void MatrixCSR::PrintDiagBlocks() const
		{
			std::ofstream myfile;
			myfile.open("test_diagValues.txt");
			sendDiagVals2Stream(myfile);
			myfile.close();
		}

	} // linear_problem
