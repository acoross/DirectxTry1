#include <iostream>
#include <string>

#include "math.h"

using Acoross::Math::SqMatrix;

void print_matrix(const SqMatrix<3>& matrix)
{
	for (int r = 0; r < 3; ++r)
	{
		for (int c = 0; c < 3; ++c)
		{
			printf("%.3f, ", matrix(r, c));
		}
		std::cout << std::endl;
	}
}

int main()
{
	SqMatrix<2> mat1{
		3, 0,
		-1, 2
	};
	auto inv2 = mat1.inverse();

	SqMatrix<3> mat{
		2, -5, 3,
		1, 3, 4,
		-2, 3, 7
	};
	std::cout << "mat: " << std::endl;
	print_matrix(mat);
	std::cout << std::endl;

	auto inv = mat.inverse();
	std::cout << "inv: " << std::endl;
	print_matrix(inv);
	std::cout << std::endl;

	auto identity = inv * mat;
	std::cout << "identity: " << std::endl;
	print_matrix(identity);
	std::cout << std::endl;

	float det = mat.determinant();
	std::cout << det << std::endl;
}