#if !defined FFT_H
#define FFT_H
//------------------------------------
//  fft.h
//  Fast Fourier Transform
//  (c) Reliable Software, 1996
//------------------------------------
#include "fftw3.h"
#include "windows.h"
#include "complex.h"
#include "assert.h"
#include <mmsyscom.h>
#include <wtypes.h>
#include <wincontypes.h>
#include <rpcasync.h>
class Thread
{
public:
    Thread(DWORD(WINAPI* pFun) (void* arg), void* pArg)
    {
        _handle = CreateThread(
            0, // Security attributes
            0, // Stack size
            pFun,
            pArg,
            CREATE_SUSPENDED,
            &_tid);
    }
    ~Thread() { CloseHandle(_handle); }
    void Resume() { ResumeThread(_handle); }
    void WaitForDeath()
    {
        WaitForSingleObject(_handle, 2000);
    }
private:
    HANDLE _handle{};
    DWORD  _tid{};     // thread id
};

class Mutex
{
    friend class Lock;
public:
    Mutex() { InitializeCriticalSection(&_critSection); }
    ~Mutex() { DeleteCriticalSection(&_critSection); }
private:
    void Acquire()
    {
        EnterCriticalSection(&_critSection);
    }
    void Release()
    {
        LeaveCriticalSection(&_critSection);
    }

    CRITICAL_SECTION _critSection;
};

class Lock
{
public:
    // Acquire the state of the semaphore
    Lock(Mutex& mutex)
        : _mutex(mutex)
    {
        _mutex.Acquire();
    }
    // Release the state of the semaphore
    ~Lock()
    {
        _mutex.Release();
    }
private:
    Mutex& _mutex;
};

class Event
{
public:
    Event()
    {
        // start in non-signaled state (red light)
        // auto reset after every Wait
        _handle = CreateEvent(0, FALSE, FALSE, 0);
    }

    ~Event()
    {
        CloseHandle(_handle);
    }

    // put into signaled state
    void Release() { SetEvent(_handle); }
    void Wait()
    {
        // Wait until event is in signaled (green) state
        WaitForSingleObject(_handle, INFINITE);
    }
    operator HANDLE () { return _handle; }
private:
    HANDLE _handle;
};

class TrafficLight
{
public:
    TrafficLight()
    {
        // Start in non-signaled state (red light)
        // Manual reset
        _handle = CreateEvent(0, TRUE, FALSE, 0);
    }

    ~TrafficLight()
    {
        CloseHandle(_handle);
    }

    // put into signaled state
    void GreenLight() { SetEvent(_handle); }

    // put into non-signaled state
    void RedLight() { ResetEvent(_handle); }

    void Wait()
    {
        // Wait until event is in signaled (green) state
        WaitForSingleObject(_handle, INFINITE);
    }

private:
    HANDLE _handle;
};

class WaveFormat : public WAVEFORMATEX
{
public:
    WaveFormat(
        WORD    nCh, // number of channels (mono, stereo)
        DWORD   nSampleRate, // sample rate
        WORD    BitsPerSample)
    {
        wFormatTag = WAVE_FORMAT_PCM;
        nChannels = nCh;
        nSamplesPerSec = nSampleRate;
        nAvgBytesPerSec = nSampleRate * nCh * BitsPerSample / 8;
        nBlockAlign = nChannels * BitsPerSample / 8;
        wBitsPerSample = BitsPerSample;
        cbSize = 0;
    }

    bool isInSupported(UINT idDev)
    {
        MMRESULT result = waveInOpen
        (0, idDev, this, 0, 0, WAVE_FORMAT_QUERY);
        return result == MMSYSERR_NOERROR;
    }
};

class WaveHeader : public WAVEHDR
{
public:
    bool IsDone() const { return dwFlags & WHDR_DONE; }
};

/*class WaveOutDevice
{
public:
    WaveOutDevice() { _status = MMSYSERR_BADDEVICEID; };
    WaveOutDevice(UINT idDev, WaveFormat& format, Event& event) { Open(idDev, format, event); };
    ~WaveOutDevice()
    {
        if (Ok())
        {
            waveOutReset(_handle);
            waveOutClose(_handle);
        }
    }

    bool    Open(UINT idDev, WaveFormat& format, Event& event);
    void    Reset() { if (Ok())waveOutReset(_handle); }
    bool    Close();
    void    Prepare(WaveHeader* pHeader) { waveOutPrepareHeader(_handle, pHeader, sizeof(WAVEHDR)); }
    void	UnPrepare(WaveHeader* pHeader) { waveOutUnprepareHeader(_handle, pHeader, sizeof(WAVEHDR)); }
    //	void    SendBuffer (WaveHeader * pHeader){waveOutAddBuffer (_handle, pHeader, sizeof(WAVEHDR));}
    bool    Ok() { return _status == MMSYSERR_NOERROR; }
    //	void    Start () { waveOutRestart(_handle); }
    //	void    Stop () { waveOutStop(_handle); }
    bool    isInUse() { return _status == MMSYSERR_ALLOCATED; }
    DWORD   GetPosSample();
    UINT    GetError() { return _status; }
    void    GetErrorText(char* buf, int len) { waveOutGetErrorText(_status, buf, len); }
    const char* GetDeviceName() { if (Ok()) return _wavOutCaps.szPname; return NULL; };

    static UINT GetNumDevs() { return waveOutGetNumDevs(); }
    bool GetDevCaps() { _status = waveOutGetDevCaps(_idDev, &_wavOutCaps, sizeof(WAVEOUTCAPS)); return Ok(); };

private:
    UINT		_idDev;
    WAVEOUTCAPS	_wavOutCaps;
    HWAVEOUT	_handle;
    MMRESULT	_status;
};
*/
class WaveInDevice
{
public:
    WaveInDevice();
    WaveInDevice(UINT idDev, WaveFormat& format, Event& event);
    ~WaveInDevice();
    bool    Open(UINT idDev, WaveFormat& format, Event& event);
    void    Reset();
    bool    Close();
    void    Prepare(WaveHeader* pHeader);
    void    UnPrepare(WaveHeader* pHeader);
    void    SendBuffer(WaveHeader* pHeader);
    bool    Ok() { return _status == MMSYSERR_NOERROR; }
    void    Start() { waveInStart(_handle); }
    void    Stop() { waveInStop(_handle); }
    bool    isInUse() { return _status == MMSYSERR_ALLOCATED; }
    unsigned int    GetError() { return _status; }
    void    GetErrorText(char* buf, int len);
private:
    HWAVEIN     _handle {};
    MMRESULT    _status {};
};

inline WaveInDevice::WaveInDevice()
{
    _status = MMSYSERR_BADDEVICEID;
}

inline WaveInDevice::WaveInDevice(
    unsigned int idDev, WaveFormat& format, Event& event)
{
    Open(idDev, format, event);
}

inline WaveInDevice::~WaveInDevice()
{
    if (Ok())
    {
        waveInReset(_handle);
        waveInClose(_handle);
    }
}

inline bool WaveInDevice::Open(
    unsigned int idDev, WaveFormat& format, Event& event)
{
    _status = waveInOpen(
        &_handle,
        idDev,
        &format,
        (DWORD_PTR) (HANDLE)event,
        0, // callback instance data
        CALLBACK_EVENT);

    return Ok();
}

inline void WaveInDevice::Reset()
{
    if (Ok())
        waveInReset(_handle);
}

inline bool WaveInDevice::Close()
{
    if (Ok() && waveInClose(_handle) == 0)
    {
        _status = MMSYSERR_BADDEVICEID;
        return TRUE;
    }
    else
        return FALSE;
}

inline void WaveInDevice::Prepare(WaveHeader* pHeader)
{
    waveInPrepareHeader(_handle, pHeader, sizeof(WAVEHDR));
}

inline void WaveInDevice::SendBuffer(WaveHeader* pHeader)
{
    waveInAddBuffer(_handle, pHeader, sizeof(WAVEHDR));
}

inline void WaveInDevice::UnPrepare(WaveHeader* pHeader)
{
    waveInUnprepareHeader(_handle, pHeader, sizeof(WAVEHDR));
}


/*inline void WaveInDevice::GetErrorText(char* buf, int len)
{
    LPWSTR lpc = R(buf);

    waveInGetErrorText(_status, , len);
}*/

class Recorder
{
    friend class SampleIter;
    enum { NUM_BUF = 8 };
public:
    Recorder(
        int cSamples,
        int cSamplePerSec,
        int nChannels,
        int bitsPerSecond);

    ~Recorder();
    BOOL    Start (Event & event);
    void    Stop (){delete[]_pBuf;};
    BOOL    BufferDone ();

    BOOL    IsBufferDone () const
    {
        return _header [_iBuf].IsDone ();
    }

    BOOL    IsStarted () const { return _isStarted; }
    int     SampleCount () const { return _cSamples; }
    int     BitsPerSample () const { return _bitsPerSample; }
    int     SamplesPerSecond () const { return _cSamplePerSec; }
protected:
    virtual int GetSample (char *pBuf, int i) const = 0;
    char * GetData () const { return _header [_iBuf].lpData; }

    BOOL            _isStarted;

    WaveInDevice    _waveInDevice;
    int             _cSamplePerSec;     // sampling frequency
    int             _cSamples;          // samples per buffer
    int             _nChannels;
    int             _bitsPerSample;
    int             _cbSampleSize;      // bytes per sample

    int             _cbBuf;             // bytes per buffer
    int             _iBuf;              // current buffer #
    char           *_pBuf;              // pool of buffers
    WaveHeader      _header[NUM_BUF] {};  // pool of headers 
};

Recorder::Recorder (
    int cSamples,
    int cSamplePerSec,
    int nChannels,
    int bitsPerSample)
: _iBuf(0),
  _cSamplePerSec (cSamplePerSec),
  _cSamples (cSamples),
  _cbSampleSize (nChannels * bitsPerSample/8),
  _cbBuf (cSamples * nChannels * bitsPerSample/8),
  _nChannels (nChannels),
  _bitsPerSample (bitsPerSample),
  _isStarted(FALSE)
{
    _pBuf = new char [(int) ( _cbBuf * NUM_BUF)];
}

Recorder::~Recorder()
{
    Stop(); 
}

class SampleIter
{
public:
    SampleIter(Recorder const& recorder);
    bool AtEnd() const { return _iCur == _iEnd; }
    void Advance() { _iCur++; }
    void Rewind() { _iCur = _iEnd - _recorder.SampleCount(); }
    int  GetSample() const
    {
        return _recorder.GetSample(_pBuffer, _iCur);
    }
    int  Count() const { return _recorder.SampleCount(); }
private:
    char* _pBuffer;
    Recorder const& _recorder;
    int         _iCur;
    int         _iEnd;
};

class Fft
{
public:
    Fft (int Points, long sampleRate);
    ~Fft ();
    int     Points () const { return _Points; }
    void    Transform ();
    void    CopyIn (SampleIter& iter);

    double  GetIntensity (int i) const
    { 
        assert (i < _Points);
        return _X[i].Mod()/_sqrtPoints; 
    }

    int     GetFrequency (int point) const
    {
        // return frequency in Hz of a given point
        assert (point < _Points);
        long x =_sampleRate * point;
        return x / _Points;
    }

    int     HzToPoint (int freq) const 
    { 
        return (long)_Points * freq / _sampleRate; 
    }

    int     MaxFreq() const { return _sampleRate; }

    int     Tape (int i) const
    {
        assert (i < _Points);
        return (int) _aTape[i];
    }

private:

    void PutAt ( int i, double val )
    {
        _X [_aBitRev[i]] = Complex (val);
    }

    int			_Points;
    long		_sampleRate;
    int			_logPoints;
    double		_sqrtPoints;
    int		   *_aBitRev;       // bit reverse vector
    Complex	   *_X;             // in-place fft array
    Complex	  **_W;             // exponentials
    double     *_aTape;         // recording tape
};

#endif
