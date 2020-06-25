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

#pragma once
#include "finalization/src/api/RemoteFinalizationApi.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace mocks {

	/// Mock finaliztion api that can be configured to return predefined data for requests, capture function parameters
	/// and throw exceptions at specified entry points.
	class MockFinalizationApi : public api::RemoteFinalizationApi {
	public:
		enum class EntryPoint {
			None,
			Messages
		};

	public:
		/// Creates a finalization api around a range of messages (\a messageRange).
		explicit MockFinalizationApi(const api::FinalizationMessageRange& messageRange)
				: api::RemoteFinalizationApi({ test::GenerateRandomByteArray<Key>(), "fake-host-from-mock-finalization-api" })
				, m_messageRange(api::FinalizationMessageRange::CopyRange(messageRange))
				, m_errorEntryPoint(EntryPoint::None)
		{}

	public:
		/// Sets the entry point where the exception should occur to \a entryPoint.
		void setError(EntryPoint entryPoint) {
			m_errorEntryPoint = entryPoint;
		}

		/// Gets a vector of parameters that were passed to the messages requests.
		const auto& messagesRequests() const {
			return m_messagesRequests;
		}

	public:
		/// Gets the configured messages and throws if the error entry point is set to Messages.
		/// \note The \a stepIdentifier and \a knownShortHashes parameters are captured.
		thread::future<api::FinalizationMessageRange> messages(
				const crypto::StepIdentifier& stepIdentifier,
				model::ShortHashRange&& knownShortHashes) const override {
			m_messagesRequests.emplace_back(stepIdentifier, std::move(knownShortHashes));
			if (shouldRaiseException(EntryPoint::Messages))
				return CreateFutureException<api::FinalizationMessageRange>("messages error has been set");

			return thread::make_ready_future(api::FinalizationMessageRange::CopyRange(m_messageRange));
		}

	private:
		bool shouldRaiseException(EntryPoint entryPoint) const {
			return m_errorEntryPoint == entryPoint;
		}

		template<typename T>
		static thread::future<T> CreateFutureException(const char* message) {
			return thread::make_exceptional_future<T>(catapult_runtime_error(message));
		}

	private:
		api::FinalizationMessageRange m_messageRange;
		EntryPoint m_errorEntryPoint;
		mutable std::vector<std::pair<crypto::StepIdentifier, model::ShortHashRange>> m_messagesRequests;
	};
}}
