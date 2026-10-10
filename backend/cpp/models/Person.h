#ifndef MEDIPRIORITY_PERSON_H
#define MEDIPRIORITY_PERSON_H

#include <string>

namespace medipriority {

/*
 * Person is an ABSTRACT base class (has a pure virtual method), shared by
 * Patient and Doctor. This is a genuine use of inheritance/polymorphism:
 * both subclasses have a name, age and gender, but describe() and
 * role() behave differently for each -- callers can hold a Person* / Person&
 * and get the correct behavior without knowing the concrete type.
 */
class Person {
protected:
    int id_;
    std::string name_;
    int age_;
    std::string gender_;

public:
    Person(int id, std::string name, int age, std::string gender)
        : id_(id), name_(std::move(name)), age_(age), gender_(std::move(gender)) {}

    virtual ~Person() = default;

    // Encapsulation: fields are private/protected, accessed only via these methods.
    int id() const { return id_; }
    const std::string &name() const { return name_; }
    int age() const { return age_; }
    const std::string &gender() const { return gender_; }

    void setName(const std::string &name) { name_ = name; }
    void setAge(int age) { age_ = age; }

    // Abstraction + polymorphism: each concrete Person type must define these.
    virtual std::string role() const = 0;
    virtual std::string describe() const = 0;
};

} // namespace medipriority

#endif // MEDIPRIORITY_PERSON_H
