/**
 * ©Copyright Lukas Mazur 2015
 */

#ifndef INC_LUKAJ_BYREGEX_HPP
#define INC_LUKAJ_BYREGEX_HPP

#include <regex>
#include <istream>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

namespace lukaj
{

template<typename charT, typename ...Args>
class basic_byregex
{
public:
    basic_byregex(const charT *str, Args&& ...args)
        : regex(str),
          args(std::forward<Args>(args)...)
    {}

    basic_byregex(const std::basic_string<charT> &str, Args&& ...args)
        : regex(str),
          args(std::forward<Args>(args)...)
    {}

    basic_byregex(const std::basic_regex<charT> &regex, Args&& ...args)
        : regex(regex),
          args(std::forward<Args>(args)...)
    {}

    basic_byregex &operator=(const basic_byregex&) = delete;

    bool extract(typename std::basic_istream<charT> &is)
    {
        std::match_results<istream_bidirectional_iterator> results;
        istream_bidirectional_iterator it(is);

        bool hasMatched = std::regex_search(it, istream_bidirectional_iterator(), results, regex, std::regex_constants::match_continuous);

        args.extract(++results.begin());

        is.putback(*it.endBuf());

        return hasMatched;
    }

public:
    class istream_bidirectional_iterator;

private:
    template<typename U, typename ...UArgs>
    struct argpack
    {
        argpack(U &first_arg, UArgs&& ...args)
            : arg(first_arg),
              next_arg(std::forward<UArgs>(args)...)
        {}

        void extract(typename std::match_results<istream_bidirectional_iterator>::const_iterator &matches)
        {
            std::basic_istringstream<charT> is(std::basic_string<charT>(matches->first, matches->second));
            is >> arg;
            next_arg.extract(++matches);
        }

        U &arg;
        argpack<UArgs...> next_arg;
    };

    template<typename U>
    struct argpack<U>
    {
        argpack(U &first_arg)
            : arg(first_arg)
        {}

        void extract(typename std::match_results<istream_bidirectional_iterator>::const_iterator &matches)
        {
            std::basic_istringstream<charT> is(std::basic_string<charT>(matches->first, matches->second));
            is >> arg;
        }

        U &arg;
    };


    const std::basic_regex<charT> regex;
    argpack<Args...> args;

public:
    class istream_bidirectional_iterator : public std::iterator<std::bidirectional_iterator_tag, charT>
    {
    public:
        istream_bidirectional_iterator()
            : buf(nullptr),
              pos(0),
              is(nullptr)
        {}

        istream_bidirectional_iterator(typename std::basic_istream<charT> &is)
            : buf(new typename decltype(buf)::element_type),
              pos(0),
              is(&is)
        {}

        istream_bidirectional_iterator(const istream_bidirectional_iterator&) = default;

        istream_bidirectional_iterator& operator=(const istream_bidirectional_iterator &other)
        {
            buf = other.buf;
            pos = other.pos;
            is = other.is;
            return *this;
        }

        ~istream_bidirectional_iterator() = default;

        friend bool operator==(const istream_bidirectional_iterator &lhs, const istream_bidirectional_iterator &rhs)
        {
            if (!lhs.is && rhs.is) {
                return rhs.is->eof();
            }
            if (lhs.is && !rhs.is) {
                return lhs.is->eof();
            }
            return lhs.is == rhs.is && lhs.pos == rhs.pos && lhs.buf == rhs.buf;
        }

        friend bool operator!=(const istream_bidirectional_iterator &lhs, const istream_bidirectional_iterator &rhs)
        {
            return !(lhs == rhs);
        }

        charT& operator*()
        {
            if (pos >= buf->size()) {
                buf->push_back(is->get());
            }
            return (*buf)[pos];
        }

        const charT& operator*() const
        {
            if (pos >= buf->size()) {
                buf->push_back(is->get());
            }
            return (*buf)[pos];
        }

        istream_bidirectional_iterator& operator++()
        {
            advancePos();
            return *this;
        }

        istream_bidirectional_iterator operator++(int)
        {
            auto tmp = *this;
            advancePos();
            return tmp;
        }

        istream_bidirectional_iterator& operator--()
        {
            pos--;
            return *this;
        }

        istream_bidirectional_iterator operator--(int)
        {
            auto tmp = *this;
            pos--;
            return tmp;
        }

        istream_bidirectional_iterator& endBuf()
        {
            pos = buf->size() - 1;
            return *this;
        }

    private:
        void advancePos()
        {
            if (buf->size() <= ++pos) {
                buf->push_back(is->get());
            } 
        }

        std::shared_ptr<std::vector<charT>> buf;
        typename decltype(buf)::element_type::size_type pos;
        typename std::basic_istream<charT> * is;
    };
};

/*template<typename charT, typename ...Args>
typename std::basic_istream<charT>& operator>>(typename std::basic_istream<charT> &is, typename const basic_byregex<charT, ...Args> &byregex)
{
    if (!byregex.extract(is)) {
        is.setstate(std::ios::failbit)
    }
    return is;
}*/

template<typename charT, typename ...Args>
basic_byregex<charT, Args...> byregex(const std::basic_string<charT> &str, Args&& ...args)
{
    return basic_byregex<charT, Args...>(std::forward<const std::basic_string<charT>>(str), std::forward<Args>(args)...);
}

template<typename charT, typename ...Args>
basic_byregex<charT, Args...> byregex(const charT *str, Args&& ...args)
{
    return basic_byregex<charT, Args...>(std::forward<const charT*>(str), std::forward<Args>(args)...);
}

template<typename charT, typename ...Args>
basic_byregex<charT, Args...> byregex(const std::basic_regex<charT> &regex, Args&& ...args)
{
    return basic_byregex<charT, Args...>(std::forward<const std::basic_regex<charT>>(regex), std::forward<Args>(args)...);
}

} // namespace lukaj

#endif //INC_LUKAJ_BYREGEX_HPP
