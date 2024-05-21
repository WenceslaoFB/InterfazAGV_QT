#ifndef USER_H
#define USER_H

#include <QString>

class User {
private:
    QString username;
    QString role;
public:
    User(const QString &username, const QString &role) : username(username), role(role) {}
    QString getUsername() const { return username; }
    QString getRole() const { return role; }
};

#endif // USER_H
