#pragma once

class noncopyable {
public:
	noncopyable(const noncopyable&) = delete;
	const noncopyable& operator=( const noncopyable& ) = delete;
protected:
	noncopyable() = default;
	~noncopyable() = default;
};