#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

#define EXCEPTION_ARGS __FILE__, __FUNCTION__, __LINE__

class BaseException : public std::exception
{
public:

    explicit BaseException(const char* file, const char* function, int line, const char* message) throw()
        : m_file(file), m_function(function), m_message(message), m_line(line) {}

    virtual ~BaseException() throw() {}

    virtual const char* file() const throw() { return m_file; }
    virtual const char* function() const throw() { return m_function; }
    virtual int line() const throw() { return m_line; }
    virtual const char* what() const throw() { return m_message; }

private:

    const char *m_file, *m_function, *m_message;
    int m_line;
};

class RangeException : public BaseException
{
public:

    explicit RangeException(const char* file, const char* function, int line, const char* message) throw()
        : BaseException(file, function, line, message) {}

    ~RangeException() throw() {}
};

class SizeException : public BaseException
{
public:

    explicit SizeException(const char* file, const char* function, int line, const char* message) throw()
        : BaseException(file, function, line, message) {}

    ~SizeException() throw() {}
};

class ParseException : public BaseException
{
public:

    explicit ParseException(const char* file, const char* function, int line, const char* message) throw()
        : BaseException(file, function, line, message) {}

    ~ParseException() throw() {}
};

class BadException : public BaseException
{
public:

    explicit BadException(const char* file, const char* function, int line, const char* message) throw()
        : BaseException(file, function, line, message) {}

    ~BadException() throw() {}
};

class QueryException : public BaseException
{
public:

    explicit QueryException(const char* file, const char* function, int line, const char* message) throw()
        : BaseException(file, function, line, message) {}

    ~QueryException() throw() {}
};

#endif
