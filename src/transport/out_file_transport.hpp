/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include "transport/transport.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/make_unique.hpp"

#include <memory>

class ReportError : noncopyable_but_movable
{
public:
    template<class F>
    static ReportError mk(F && f)
    {
        return ReportError(new FuncImpl<typename std::decay<F>::type>{std::forward<F>(f)});
    }

    static ReportError mk(std::nullptr_t = nullptr)
    {
        // disable allocation/deallocation
        struct NullImpl : ImplBase
        {
            void operator delete(void *) {}
            Error get_error(Error err) override { return err; }
        };
        static NullImpl null_impl;
        return ReportError(&null_impl);
    }

    Error get_error(Error err)
    {
        return this->impl->get_error(err);
    }

    Error operator()(Error err)
    {
        return this->get_error(err);
    }

private:
    struct ImplBase
    {
        virtual Error get_error(Error err) = 0;
        virtual ~ImplBase() = default;
    };

    template<class F>
    struct FuncImpl : ImplBase
    {
        F fun;
        template<class Fu>
        FuncImpl(Fu && f) : fun(std::forward<Fu>(f)) {}
        Error get_error(Error err) override { return fun(err); }
    };

    template<class F, class Fu>
    static ReportError dispath_mk(Fu && f, std::false_type = typename std::is_pointer<F>::type{})
    {
        return ReportError{{new FuncImpl<F>{std::forward<Fu>(f)}}};
    }

    template<class F, class Fu>
    static ReportError dispath_mk(Fu && f, std::true_type = typename std::is_pointer<F>::type{})
    {
        return f ? ReportError(new FuncImpl<F>{f}) : mk(nullptr);
    }

    ReportError(ImplBase* p)
    : impl(p)
    {}

    std::unique_ptr<ImplBase> impl;
};

inline ReportError auth_report_error(auth_api& auth)
{
    return ReportError::mk([&auth](Error error) {
        if (error.id == ENOSPC) {
            auth.report("FILESYSTEM_FULL", "100|unknow");
            error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
        }
        return error;
    });
}

inline ReportError auth_report_error(auth_api* auth)
{
    return auth ? auth_report_error(*auth) : ReportError::mk([](Error error){
        if (error.id == ENOSPC) {
            LOG(LOG_ERR, "FILESYSTEM_FULL");
            error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
        }
        return error;
    });
}

struct OutFileTransport : Transport
{
    explicit OutFileTransport(unique_fd fd, ReportError report_error = ReportError::mk()) noexcept
    : file(std::move(fd))
    , report_error(std::move(report_error))
    {}

    bool disconnect() override
    {
        return this->file.close();
    }

    void seek(int64_t offset, int whence) override
    {
        if (lseek64(this->file.fd(), offset, whence) == static_cast<off_t>(-1)) {
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }
    }

    int get_fd() const override
    {
        return this->file.fd();
    }

    void open(unique_fd fd)
    {
        this->file = std::move(fd);
    }

    bool is_open() const
    {
        return this->file.is_open();
    }

    // alias on disconnect
    void close()
    {
        this->file.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override
    {
        size_t total_sent = 0;
        while (total_sent != len) {
            ssize_t const ret = ::write(this->file.fd(), data + total_sent, len - total_sent);
            if (ret <= 0){
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
            total_sent += ret;
        }
        this->last_quantum_sent += total_sent;
        if (total_sent != len) {
            this->status = false;
            throw this->report_error(Error(ERR_TRANSPORT_WRITE_FAILED, errno));
        }
    }

    unique_fd file;
    ReportError report_error;
};
