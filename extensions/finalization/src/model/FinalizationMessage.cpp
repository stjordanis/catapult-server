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

#include "FinalizationMessage.h"
#include "FinalizationContext.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/Sortition.h"
#include "catapult/crypto_voting/OtsTree.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include "catapult/utils/MemoryUtils.h"

namespace catapult { namespace model {

#define DEFINE_ENUM ProcessMessageResult
#define ENUM_LIST PROCESS_MESSAGE_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	namespace {
#pragma pack(push, 1)

		struct SortitionVrfInput {
			catapult::GenerationHash GenerationHash;
			crypto::StepIdentifier StepIdentifier;
		};

#pragma pack(pop)

		RawBuffer ToBuffer(const SortitionVrfInput& input) {
			return { reinterpret_cast<const uint8_t*>(&input), sizeof(SortitionVrfInput) };
		}

		RawBuffer ToBuffer(const FinalizationMessage& message) {
			return {
				reinterpret_cast<const uint8_t*>(&message) + FinalizationMessage::Header_Size,
				message.Size - FinalizationMessage::Header_Size
			};
		}
	}

	// TODO: FinalizationContext::lookup expects BLS key, but, for now, interpret it as ED25519 key

	std::unique_ptr<FinalizationMessage> PrepareMessage(
			crypto::OtsTree& otsTree,
			const crypto::KeyPair& vrfKeyPair,
			const crypto::StepIdentifier& stepIdentifier,
			const model::HashRange& hashes,
			const FinalizationContext& context) {
		auto accountView = context.lookup(otsTree.rootPublicKey().copyTo<VotingKey>());
		if (Amount() == accountView.Weight)
			return nullptr;

		// 1. generate sortition hash proof
		SortitionVrfInput sortitionVrfInput{ context.generationHash(), stepIdentifier };
		auto sortitionHashProof = crypto::GenerateVrfProof(ToBuffer(sortitionVrfInput), vrfKeyPair);
		auto sortitionHash = crypto::GenerateVrfProofHash(sortitionHashProof.Gamma);

		// 2. check selection
		auto numVotes = crypto::Sortition(sortitionHash, context.config().Size, accountView.Weight, context.weight());
		if (0 == numVotes)
			return nullptr;

		// 3. create message and copy hashes
		auto numHashes = static_cast<uint32_t>(hashes.size());
		uint32_t messageSize = sizeof(FinalizationMessage) + numHashes * Hash256::Size;

		auto pMessage = utils::MakeUniqueWithSize<FinalizationMessage>(messageSize);
		pMessage->Size = messageSize;
		pMessage->HashesCount = numHashes;
		pMessage->StepIdentifier = stepIdentifier;
		pMessage->SortitionHashProof = sortitionHashProof;

		auto* pHash = pMessage->HashesPtr();
		for (const auto& hash : hashes)
			*pHash++ = hash;

		// 4. sign
		pMessage->Signature = otsTree.sign(pMessage->StepIdentifier, ToBuffer(*pMessage));
		return pMessage;
	}

	std::pair<ProcessMessageResult, size_t> ProcessMessage(const FinalizationMessage& message, const FinalizationContext& context) {
		auto accountView = context.lookup(message.Signature.Root.ParentPublicKey.copyTo<VotingKey>());
		if (Amount() == accountView.Weight)
			return std::make_pair(ProcessMessageResult::Failure_Voter, 0);

		SortitionVrfInput sortitionVrfInput{ context.generationHash(), message.StepIdentifier };
		auto sortitionHash = crypto::VerifyVrfProof(message.SortitionHashProof, ToBuffer(sortitionVrfInput), accountView.VrfPublicKey);
		if (Hash512() == sortitionHash)
			return std::make_pair(ProcessMessageResult::Failure_Sortition_Hash_Proof, 0);

		auto numVotes = crypto::Sortition(sortitionHash, context.config().Size, accountView.Weight, context.weight());
		if (0 == numVotes)
			return std::make_pair(ProcessMessageResult::Failure_Selection, 0);

		if (!crypto::Verify(message.Signature, message.StepIdentifier, ToBuffer(message)))
			return std::make_pair(ProcessMessageResult::Failure_Message_Signature, 0);

		return std::make_pair(ProcessMessageResult::Success, numVotes);
	}
}}