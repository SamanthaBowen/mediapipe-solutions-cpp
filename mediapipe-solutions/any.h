// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <typeindex>

#include "mediapipe/framework/packet.h"

namespace mediapipe_solutions {
	class Any {
		public:
			template <typename T>
			static Any Adopt(std::unique_ptr<T> ptr);

			constexpr Any();

			constexpr Any(const Any &other) = delete;
			Any(Any &&other);

			explicit Any(const mediapipe::Packet &other);
			explicit Any(mediapipe::Packet &&other);

			template <typename T>
			Any(const T &other);

			template <typename T>
			Any(T &other);

			template <typename T>
			Any(T &&other);

			~Any();

			operator mediapipe::Packet() const&;
			explicit operator mediapipe::Packet() &&;

			mediapipe::Packet At(mediapipe::Timestamp timestamp) const&;
			mediapipe::Packet At(mediapipe::Timestamp timestamp) &&;

			template <typename T>
			const T &Get() const&;
		private:
			std::shared_ptr<mediapipe::packet_internal::HolderBase> holder_;

			Any(std::shared_ptr<mediapipe::packet_internal::HolderBase> holder);

			template <typename T>
			Any(std::shared_ptr<mediapipe::packet_internal::Holder<T>> holder);
	};

	template <typename T>
	Any Any::Adopt(std::unique_ptr<T> ptr) {
		return Any(std::make_shared<mediapipe::packet_internal::Holder<T>>(ptr.release()));
	}

	inline constexpr Any::Any() :
		holder_(nullptr) {
	}

	inline Any::Any(Any &&other) :
		holder_(std::exchange(other.holder_, nullptr)) {
	}

	inline Any::Any(std::shared_ptr<mediapipe::packet_internal::HolderBase> holder) :
		holder_(std::move(holder)) {
	}

	template <typename T>
	inline Any::Any(std::shared_ptr<mediapipe::packet_internal::Holder<T>> holder) :
		holder_(std::move(holder)) {
	}

	inline Any::Any(const mediapipe::Packet &other) :
		holder_(mediapipe::packet_internal::GetHolderShared(other)) {
	}

	inline Any::Any(mediapipe::Packet &&other) :
		holder_(mediapipe::packet_internal::GetHolderShared(std::move(other))) {
	}

	template <typename T>
	inline Any::Any(const T &other) :
		holder_(std::make_shared<mediapipe::packet_internal::Holder<T>>(new T(other))) {
	}

	template <typename T>
	inline Any::Any(T &other) :
		Any((const T &)(other)) {
	}

	template <typename T>
	inline Any::Any(T &&other) :
		holder_(std::make_shared<mediapipe::packet_internal::Holder<T>>(new T(std::move(other)))) {
	}

	inline Any::~Any() {
	}

	inline Any::operator mediapipe::Packet() const& {
		return At(mediapipe::Timestamp());
	}
	
	inline Any::operator mediapipe::Packet() && {
		return std::move(*this).At(mediapipe::Timestamp());
	}

	inline mediapipe::Packet Any::At(mediapipe::Timestamp timestamp) const& {
		return mediapipe::packet_internal::Create(holder_, timestamp);
	}

	inline mediapipe::Packet Any::At(mediapipe::Timestamp timestamp) && {
		return mediapipe::packet_internal::Create(std::move(holder_), timestamp);
	}

	template <typename T>
	inline const T &Any::Get() const& {
		return holder_->As<T>()->data();
	}
}
