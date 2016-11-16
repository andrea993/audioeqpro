#ifndef __Q_PAMOD
#define __Q_PAMOD

#include<QString>
#include<exception>

namespace qpa
{
class PulseAudioException : public std::exception 
{
public:
    explicit PulseAudioException(const char* strerr);
    virtual const char* what() const throw();
private:
    const char* str_err;
};

void loadModule(const QString& name,const QString& args);
}

#endif
