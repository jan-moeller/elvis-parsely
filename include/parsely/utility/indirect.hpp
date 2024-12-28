//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#ifndef INDIRECT_HPP
#define INDIRECT_HPP

#include <memory>

namespace parsely
{
// Stores a T on the heap and provides regular value copy & move semantics. This indirect implementation is nullable.
//
// Note that the assignment from nullptr isn't super sound as it would be ambiguous in cases like indirect<nullptr_t>.
// But those cases don't arise within elvis parsely, so this doesn't matter.
template<typename T>
class indirect
{
  public:
    constexpr indirect()
        : indirect(nullptr)
    {
    }
    constexpr explicit indirect(std::nullptr_t)
        : m_ptr(std::unique_ptr<T>{})
    {
    }
    explicit constexpr indirect(std::unique_ptr<T> value)
        : m_ptr(std::move(value))
    {
    }
    constexpr /* implicit */ indirect(T value) // NOLINT(*-explicit-constructor)
        : indirect(std::make_unique<T>(std::move(value)))
    {
    }
    template<typename... Args>
    explicit constexpr indirect(std::in_place_t /*unused*/, Args&&... args)
        : indirect(std::make_unique<T>(std::forward<Args>(args)...))
    {
    }

    constexpr indirect(indirect const& other)
        : indirect(std::make_unique<T>(*other))
    {
    }

    constexpr indirect(indirect&& other) noexcept
        : indirect(std::move(other.m_ptr))
    {
    }

    constexpr auto operator=(indirect const& other) -> indirect&
    {
        if (static_cast<bool>(*this) && static_cast<bool>(other))
            *m_ptr = *other;
        else if (static_cast<bool>(other))
            m_ptr = std::make_unique<T>(*other);
        else
            m_ptr = nullptr;
        return *this;
    }

    constexpr auto operator=(indirect&& other) noexcept -> indirect&
    {
        swap(other);
        return *this;
    }

    constexpr auto operator=(nullptr_t) -> indirect&
    {
        m_ptr = nullptr;
        return *this;
    }

    constexpr auto operator=(T other) -> indirect&
    {
        if (static_cast<bool>(*this))
            *m_ptr = std::move(other);
        else
            m_ptr = std::make_unique<T>(std::move(other));
        return *this;
    }

    constexpr ~indirect() = default;

    constexpr void swap(indirect& other) noexcept { m_ptr.swap(other.m_ptr); }

    [[nodiscard]] constexpr explicit operator bool() const noexcept { return m_ptr != nullptr; }

    constexpr auto operator*() const& -> T const& { return *m_ptr; }
    constexpr auto operator*() const&& -> T const&& { return std::move(*m_ptr); }
    constexpr auto operator*() & -> T& { return *m_ptr; }
    constexpr auto operator*() && -> T&& { return std::move(*m_ptr); }

    constexpr auto operator->() const -> T const* { return &*m_ptr; }
    constexpr auto operator->() -> T* { return &*m_ptr; }

    constexpr auto operator==(indirect const& other) const -> bool
    {
        if (!static_cast<bool>(*this))
            return !static_cast<bool>(other);
        if (!static_cast<bool>(other))
            return !static_cast<bool>(*this);
        return *m_ptr == *other.m_ptr;
    }

    constexpr auto operator==(std::nullptr_t) const -> bool { return !static_cast<bool>(*this); }
    constexpr auto operator==(T const& other) const -> bool { return static_cast<bool>(*this) && *m_ptr == other; }

  private:
    std::unique_ptr<T> m_ptr;
};
} // namespace parsely

#endif // INDIRECT_HPP
