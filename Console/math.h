#pragma once

#include <array>

namespace Acoross
{
	namespace Math
	{
		// ¼ÒÇà·Ä
		template<typename TMat, size_t TOrder = TMat::Order() - 1>
		class MinorMatrix
		{
		public:
			typedef MinorMatrix<TMat, TOrder> MyMatrixType;

			MinorMatrix(TMat& mat, size_t exclude_r, size_t exclude_c)
				: matrix(mat), exclude_r(exclude_r), exclude_c(exclude_c)
			{}

			static constexpr size_t Order()
			{
				return TOrder;
			}

			float& operator()(size_t row, size_t col)
			{
				if (row >= exclude_r)
					++row;
				if (col >= exclude_c)
					++col;

				return matrix.operator()(row, col);
			}

			float determinant()
			{
				if (Order() == 1)
					return operator()(0, 0);

				float sum = 0;

				for (size_t i = 0; i < Order(); ++i)
				{
					MinorMatrix<MyMatrixType> minMat(*this, 0, i);
					float det = minMat.determinant();
					int cof = 1;
					if (i % 2 == 1)
						cof = -1;

					sum += det * operator()(0, i) * cof;
				}

				return sum;
			}

			const size_t exclude_r;
			const size_t exclude_c;

		private:
			TMat& matrix;
		};

		template<typename TMat>
		class MinorMatrix<TMat, 0>
		{
		public:
			MinorMatrix(TMat& mat, size_t exclude_r, size_t exclude_c)
			{}

			float determinant()
			{
				return 1;
			}
		};

		template<size_t TOrder>
		class SqMatrix
		{
		public:
			typedef SqMatrix<TOrder> MyMatrixType;

			// non-copyable
			SqMatrix(SqMatrix& rhs) = delete;
			SqMatrix& operator=(SqMatrix& rhs) = delete;

			// ctor.
			SqMatrix()
				: _Elems(new std::array<float, TOrder * TOrder>{0, })
			{}

			template <typename... TArgs>
			SqMatrix(TArgs&&... il)
				: _Elems(new std::array<float, TOrder * TOrder>{(float)il...})
			{
			}

			// movable
			SqMatrix(SqMatrix&& rhs)
				: _Elems(rhs._Elems)
			{
				rhs._Elems = nullptr;
			}
			SqMatrix& operator=(SqMatrix&& rhs)
			{
				std::swap(_Elems, rhs._Elems);
				return *this;
			}

			// dtor.
			~SqMatrix()
			{
				if (_Elems != nullptr)
				{
					delete[] _Elems;
					_Elems = nullptr;
				}
			}

			static constexpr size_t Order()
			{
				return TOrder;
			}

			float& operator()(size_t row, size_t col)
			{
				return (*_Elems)[row * TOrder + col];
			}

			const float& operator()(size_t row, size_t col) const
			{
				return (*_Elems)[row * TOrder + col];
			}

			float determinant()
			{
				if (Order() == 1)
					return operator()(0, 0);

				float sum = 0;

				for (size_t i = 0; i < Order(); ++i)
				{
					MinorMatrix<MyMatrixType> minMat(*this, 0, i);
					float det = minMat.determinant();
					int cof = 1;
					if (i % 2 == 1)
						cof = -1;

					sum += det * operator()(0, i) * cof;
				}

				return sum;
			}

			MyMatrixType adjoint()
			{
				MyMatrixType adj;

				for (int r = 0; r < TOrder; ++r)
				{
					for (int c = 0; c < TOrder; ++c)
					{
						MinorMatrix<MyMatrixType> minMat(*this, r, c);
						auto det = minMat.determinant();
						if ((r + c) % 2 == 0)
							adj(c, r) = det;
						else
							adj(c, r) = -1 * det;
					}
				}

				return adj;
			}

			MyMatrixType inverse()
			{
				auto adj = adjoint();
				auto det = determinant();
				return adj / det;
			}

			SqMatrix<TOrder> operator * (float num) const
			{
				SqMatrix<TOrder> retMatrix{ 0, };

				for (int r = 0; r < TOrder; ++r)
				{
					for (int c = 0; c < TOrder; ++c)
					{
						retMatrix(r, c) = (*this)(r, c) * num;
					}
				}

				return retMatrix;
			}

			SqMatrix<TOrder> operator / (float num) const
			{
				if (num == 0)
					throw std::exception("divided by zero");

				SqMatrix<TOrder> retMatrix{ 0, };

				for (int r = 0; r < TOrder; ++r)
				{
					for (int c = 0; c < TOrder; ++c)
					{
						retMatrix(r, c) = (*this)(r, c) / num;
					}
				}

				return retMatrix;
			}

			SqMatrix<TOrder> operator * (const SqMatrix<TOrder>& rhs) const
			{
				SqMatrix<TOrder> retMatrix{ 0, };

				for (int r = 0; r < TOrder; ++r)
				{
					for (int c = 0; c < TOrder; ++c)
					{
						for (int i = 0; i < TOrder; ++i)
						{
							retMatrix(r, c) += (*this)(r, i) * rhs(i, c);
						}
					}
				}

				return retMatrix;
			}

		private:
			std::array<float, TOrder * TOrder>* _Elems = nullptr;
		};
	}
}
