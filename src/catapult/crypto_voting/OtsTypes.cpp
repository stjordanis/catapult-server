/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "OtsTypes.h"

namespace catapult { namespace crypto {

	// region ots options

	bool OtsOptions::operator==(const OtsOptions& rhs) const {
		return MaxRounds == rhs.MaxRounds && MaxSubRounds == rhs.MaxSubRounds;
	}

	bool OtsOptions::operator!=(const OtsOptions& rhs) const {
		return !(*this == rhs);
	}

	// endregion

	// region step identifier

	bool StepIdentifier::operator==(const StepIdentifier& rhs) const {
		return Point == rhs.Point && Round == rhs.Round && SubRound == rhs.SubRound;
	}

	bool StepIdentifier::operator!=(const StepIdentifier& rhs) const {
		return !(*this == rhs);
	}

	bool StepIdentifier::operator<(const StepIdentifier& rhs) const {
		return Point < rhs.Point
				|| (Point == rhs.Point && Round < rhs.Round)
				|| (Point == rhs.Point && Round == rhs.Round && SubRound < rhs.SubRound);
	}

	bool StepIdentifier::operator<=(const StepIdentifier& rhs) const {
		return *this < rhs || *this == rhs;
	}

	bool StepIdentifier::operator>(const StepIdentifier& rhs) const {
		return !(*this <= rhs);
	}

	bool StepIdentifier::operator>=(const StepIdentifier& rhs) const {
		return !(*this < rhs);
	}

	std::ostream& operator<<(std::ostream& out, const StepIdentifier& stepIdentifier) {
		out << "(" << stepIdentifier.Point << ", " << stepIdentifier.Round << ", " << stepIdentifier.SubRound << ")";
		return out;
	}

	// endregion

	// region ots tree signature

	bool OtsTreeSignature::operator==(const OtsTreeSignature& rhs) const {
		return 0 == std::memcmp(this, &rhs, sizeof(OtsTreeSignature));
	}

	bool OtsTreeSignature::operator!=(const OtsTreeSignature& rhs) const {
		return !(*this == rhs);
	}

	// endregion
}}
