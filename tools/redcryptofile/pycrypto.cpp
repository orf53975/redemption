#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <memory>

// this is to silent warning as Python.h will redefine this constant
#undef _XOPEN_SOURCE
// this is to silent warning as Python.h will redefine this constant
#undef _POSIX_C_SOURCE
#include "Python.h"

#include <algorithm>
#include <unistd.h>
#include <genrandom.hpp>
#include <new>

#include "fdbuf.hpp"
#include "filter/crypto_filter.hpp"

#include "cryptofile.hpp"

struct crypto_file_read
{
  transfil::decrypt_filter decrypt;
  io::posix::fdbuf file;
  crypto_file_read(int fd) : file(fd) {}
};

struct crypto_file_write
{
  transfil::encrypt_filter encrypt;
  io::posix::fdbuf file;
  crypto_file_write(int fd) : file(fd) {}
};

enum crypto_type {
    CRYPTO_DECRYPT_TYPE,
    CRYPTO_ENCRYPT_TYPE
};

extern "C" {

int gl_read_nb_files = 0;
struct crypto_file_read * gl_file_store_read[1024];
int gl_write_nb_files = 0;
struct crypto_file_write * gl_file_store_write[1024];

unsigned char iv[32] = {0}; //  not used for reading
}


struct crypto_file
{
    enum crypto_type type;
    int idx;
    crypto_file(): type(CRYPTO_DECRYPT_TYPE), idx(-1) {}


    crypto_file(crypto_type t, int fd)
    : type(t)
    , idx(-1)
    {
        switch (t){
        case CRYPTO_DECRYPT_TYPE:
        {
            auto cf = new crypto_file_read(fd);
            int idx = 0;
            for (; idx < gl_read_nb_files ; idx++){
                if (gl_file_store_read[idx] == nullptr){
                    break;
                }
            }
            gl_read_nb_files += 1;
            gl_file_store_read[idx] = cf;
            this->idx = idx;
        }
        break;
        case CRYPTO_ENCRYPT_TYPE:
        {
            auto cf = new crypto_file_write(fd);
            int idx = 0;
            for (; idx < gl_write_nb_files ; idx++){
                if (gl_file_store_write[idx] == nullptr){
                    break;
                }
            }
            gl_write_nb_files += 1;
            gl_file_store_write[idx] = cf;
            this->idx = idx;
        }
        break;
        }
    }
};

extern "C" {
int gl_nb_files = 0;
struct crypto_file gl_file_store[1024];
}


extern "C" {
    UdevRandom * get_rnd();
    CryptoContext * get_cctx();
}

/* File format V1:  ([...] represent an uint32_t)
 *
 * Header:
 *  [WABCRYPTOFILE_MAGIC][FILE_VERSION][Crypto IV]
 *
 * Chunk:
 *  [ciphered chunk size (size it takes on disk)][data]
 *
 * Footer:
 *  [WABCRYPTOFILE_EOF_MAGIC][raw_file_size]
 *
 */


UdevRandom * get_rnd(){
    static UdevRandom * rnd = nullptr;
    if (rnd == nullptr){
        rnd = new UdevRandom;
    }
    return rnd;
}

Inifile * get_ini(){
    static Inifile * ini = nullptr;
    if (ini == nullptr){
        ini = new Inifile;
    }
    return ini;
}


CryptoContext * get_cctx()
{
    static CryptoContext * cctx = nullptr;
    if (cctx == nullptr){
        cctx = new CryptoContext(*get_rnd(), *get_ini());
    }
    return cctx;
}


extern "C" {

static PyObject *python_redcryptofile_open(PyObject* self, PyObject* args)
{
    char *path = nullptr;
    char *omode = nullptr;
    if (!PyArg_ParseTuple(args, "ss", &path, &omode))
        return nullptr;
    unsigned char derivator[DERIVATOR_LENGTH];
    get_cctx()->get_derivator(path, derivator, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    if (get_cctx()->compute_hmac(trace_key, derivator) == -1){
        return nullptr;
    }

    if (omode[0] == 'r') {
        int system_fd = open(path, O_RDONLY, 0600);
        if (system_fd == -1){
            printf("failed opening=%s\n", path);
            return nullptr;
        }

        auto result = crypto_file(CRYPTO_DECRYPT_TYPE, system_fd);
        auto cfr = gl_file_store_read[result.idx];
        cfr->decrypt.open(cfr->file, trace_key);

        int fd = 0;
        for (; fd < gl_nb_files ; fd++){
            if (gl_file_store[fd].idx == -1){
                break;
            }
        }
        gl_nb_files += 1;
        gl_file_store[fd] = result;
        return Py_BuildValue("i", fd);

    } else if (omode[0] == 'w') {
        unsigned i = 0;
        for (i = 0; i < sizeof(iv) ; i++){ iv[i] = i; }

        int system_fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0600);
        if (system_fd == -1){
            printf("failed opening=%s\n", path);
            return nullptr;
        }

        auto result = crypto_file(CRYPTO_ENCRYPT_TYPE, system_fd);
        auto cfw = gl_file_store_write[result.idx];
        cfw->encrypt.open(cfw->file, trace_key, get_cctx(), iv);

        int fd = 0;
        for (; fd < gl_nb_files ; fd++){
            if (gl_file_store[fd].idx == -1){
                break;
            }
        }
        gl_nb_files += 1;
        gl_file_store[fd] = result;
        return Py_BuildValue("i", fd);
    } else {
        return nullptr;
    }

    return Py_BuildValue("i", -1);
}

static PyObject *python_redcryptofile_flush(PyObject* self, PyObject* args)
{
    int fd = 0;
    if (!PyArg_ParseTuple(args, "i", &fd)){
        return nullptr;
    }
    if (fd >= gl_nb_files){
        return nullptr;
    }
    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type == CRYPTO_ENCRYPT_TYPE){
        auto & cfw = gl_file_store_write[cf.idx];
        result = cfw->encrypt.flush(cfw->file);
    }
    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_close(PyObject* self, PyObject* args)
{
    int fd = 0;
    unsigned char hash[MD_HASH_LENGTH<<1];
    char hash_digest[(MD_HASH_LENGTH*4)+1];

    if (!PyArg_ParseTuple(args, "i", &fd))
        return nullptr;

    if (fd >= static_cast<int>(sizeof(gl_file_store)/sizeof(gl_file_store[0]))){
        return nullptr;
    }

    auto & cf = gl_file_store[fd];
    if (cf.idx == -1){
        return nullptr;
    }

    int result = 0;
    switch (cf.type){
    case CRYPTO_DECRYPT_TYPE:
    {
        auto cfr = gl_file_store_read[cf.idx];
        gl_file_store_read[cf.idx] = nullptr;
        gl_read_nb_files--;
        delete cfr;
    }
    break;
    case CRYPTO_ENCRYPT_TYPE:
    {
        auto cfw = gl_file_store_write[cf.idx];
        gl_file_store_write[cf.idx] = nullptr;
        gl_write_nb_files--;
        result = cfw->encrypt.close(cfw->file, hash, get_cctx()->hmac_key);
        delete cfw;
    }
    break;
    }

    cf.idx = -1;
    gl_nb_files--;

    // Crazy API: return error as integer or HASH as string... change that
    if (result){
        return Py_BuildValue("i", result);
    }

    int idx = 0;
    for (idx = 0; idx < MD_HASH_LENGTH; idx++) {
        sprintf(hash_digest + idx * 2, "%02x", hash[idx]);
    }
    for (idx = MD_HASH_LENGTH; idx < (MD_HASH_LENGTH*2); idx++) {
        sprintf(hash_digest + idx * 2, "%02x", hash[idx]);
    }
    hash_digest[MD_HASH_LENGTH*4] = 0;
    return Py_BuildValue("s", hash_digest);
}

static PyObject *python_redcryptofile_write(PyObject* self, PyObject* args)
{
    int fd;
    PyObject *python_buf;
    if (!PyArg_ParseTuple(args, "iS", &fd, &python_buf)){
        return nullptr;
    }

    int buf_len = PyString_Size(python_buf);
    if (buf_len > 2147483647 || buf_len < 0){
        return Py_BuildValue("i", -1);
    }
    char *buf = PyString_AsString(python_buf);

    if (fd >= gl_nb_files){
        return nullptr;
    }

    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type == CRYPTO_ENCRYPT_TYPE){
        auto & cfw = gl_file_store_write[cf.idx];
        cfw->encrypt.write(cfw->file, buf, buf_len);
    }

    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_read(PyObject* self, PyObject* args)
{
    int fd;
    int buf_len;

    if (!PyArg_ParseTuple(args, "ii", &fd, &buf_len))
        return nullptr;
    if (buf_len > 2147483647 || buf_len <= 0){
        return Py_BuildValue("i", -1);
    }

    if (fd >= gl_nb_files){
        return nullptr;
    }

    std::unique_ptr<char[]> buf(new char[buf_len]);

    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type ==  CRYPTO_DECRYPT_TYPE) {
        auto & cfr = gl_file_store_read[cf.idx];
        result = cfr->decrypt.read(cfr->file, buf.get(), buf_len);
    }
    if (result < 0){
        return nullptr;
    }
    return PyString_FromStringAndSize(buf.get(), result);
}

static PyMethodDef redcryptoFileMethods[] = {
    {"open", python_redcryptofile_open, METH_VARARGS, ""},
    {"flush", python_redcryptofile_flush, METH_VARARGS, ""},
    {"close", python_redcryptofile_close, METH_VARARGS, ""},
    {"write", python_redcryptofile_write, METH_VARARGS, ""},
    {"read", python_redcryptofile_read, METH_VARARGS, ""},
    {nullptr, nullptr, 0, nullptr}
};

PyMODINIT_FUNC initredcryptofile(void)
{
    Py_InitModule("redcryptofile", redcryptoFileMethods);

    const unsigned char HASH_DERIVATOR[] = { 0x95, 0x8b, 0xcb, 0xd4, 0xee, 0xa9, 0x89, 0x5b };

    if (-1 == get_cctx()->compute_hmac(get_cctx()->hmac_key, HASH_DERIVATOR)){
        //TODO: we should LOG something here
    }
    OpenSSL_add_all_digests();
    size_t idx = 0;
    for (; idx < sizeof(gl_file_store)/sizeof(gl_file_store[0]);idx++)
    {
        gl_file_store[idx].idx = -1;
    }
    size_t idxr = 0;
    for (; idxr < sizeof(gl_file_store_read)/sizeof(gl_file_store_read[0]);idxr++)
    {
        gl_file_store_read[idxr] = nullptr;
    }
    size_t idxw = 0;
    for (; idxw < sizeof(gl_file_store_write)/sizeof(gl_file_store_write[0]);idxw++)
    {
        gl_file_store_write[idxw] = nullptr;
    }
}

}
