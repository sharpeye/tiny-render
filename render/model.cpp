#include "stdafx.h"
#include <tiny-render/model.h>

namespace sharpeye
{
	bool operator == ( Model::Face const & lhs, Model::Face const & rhs )
	{
		return lhs.v == rhs.v && lhs.t == rhs.t && lhs.n == rhs.n;
	}

} // ns sharpeye
