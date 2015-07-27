#ifndef INSTANCE_ID_H__
#define INSTANCE_ID_H__

#include <iosfwd>

template <typename Class>
class InstanceId
{
public:
    typedef unsigned int Value;

  InstanceId(): value(++Last ? Last : ++Last) {}

    operator Value() const { return value; }
    bool operator ==(const InstanceId &o) const { return value == o.value; }
    bool operator !=(const InstanceId &o) const { return !(*this == o); }
    void change() {value = ++Last ? Last : ++Last;}

    /// prints Prefix followed by ID value; \todo: use HEX for value printing?
    std::ostream &print(std::ostream &os) const;

  public:
    static const char *prefix; ///< Class shorthand string for debugging
    Value value; ///< instance identifier

  private:
    InstanceId(const InstanceId& right); ///< not implemented; IDs are unique
    InstanceId& operator=(const InstanceId &right); ///< not implemented

  private:
    static Value Last; ///< the last used ID value
};

template <typename Class>
inline std::ostream &operator <<(std::ostream &os, const InstanceId<Class> &id)
{
    return id.print(os);
}



/// convenience macro to instantiate Class-specific stuff in .cc files
#define InstanceIdDefinitions(Class, Prefix)				\
    template<> const char *InstanceId<Class>::prefix = Prefix;		\
    template<> InstanceId<Class>::Value InstanceId<Class>::Last = 0;	\
    template<> std::ostream &InstanceId<Class>::print(std::ostream &os) const {	\
	return os << prefix << value;					\
    }


#endif /* INSTANCE_ID_H__ */
