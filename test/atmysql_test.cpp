#include <tuple>
#include <iostream>
#include <chrono>
#include <any>
#include <mysqlx/xdevapi.h>
#include "gtest/gtest.h"

#include "pwd.h"

import atmysql;

struct Person {
private:
	int private_a_;
public:
	int id_;
	std::string name_;
	int age_;
	std::optional<double> salary_;
	std::chrono::system_clock::time_point born_;

	auto& set_id(const auto& value) { id_ = value; return *this; } auto& id() const { return id_; }
	auto& set_name(const auto& value) { name_ = value; return *this; } auto& name() const { return name_; }
	auto& set_age(const auto& value) { age_ = value; return *this; } auto& age() const { return age_; }
	auto& set_salary(const auto& value) { salary_ = value; return *this; } auto& salary() const { return salary_; }
	auto& set_born(const auto& value) { born_ = value; return *this; } auto& born() const { return born_; }
	auto& private_a(const auto& value) { private_a_ = value; return *this; } auto& private_a() const { return private_a_; }
	void operator=(const std::map<std::string, std::any>& obj) {
		for (const auto& [key, any_value] : obj) {
			if (key == "id") { if (any_value.type() == typeid(int)) { id_ = std::any_cast<int>(any_value); } }
			else if (key == "name") { if (any_value.type() == typeid(std::string)) { name_ = std::any_cast<std::string>(any_value); } }
			else if (key == "age") { if (any_value.type() == typeid(int)) { age_ = std::any_cast<int>(any_value); } }
			else if (key == "salary") { if (any_value.type() == typeid(std::optional<double>)) { salary_ = std::any_cast<std::optional<double>>(any_value); } }
			else if (key == "born") { if (any_value.type() == typeid(std::chrono::system_clock::time_point)) { born_ = std::any_cast<std::chrono::system_clock::time_point>(any_value); } }
			else if (key == "private_a") { if (any_value.type() == typeid(decltype(private_a_))) { private_a_ = std::any_cast<decltype(private_a_)>(any_value); } }
		}
	}
	std::map<std::string, std::any> ToOM() const {
		std::map<std::string, std::any> ret;
		ret["id"] = id();
		ret["name"] = name();
		ret["age"] = age();
		ret["salary"] = salary();
		ret["born"] = born();
		ret["private_a"] = private_a_;
		static_assert(sizeof(Person) == 72, "Member update check");
		return ret;
	}
	Person() {}
	Person(const std::map<std::string, std::any>& obj) {
		*this = obj;
	}
};

atmysql::Session sess({ "192.168.0.175" ,pwd() }, "bdmysql");

TEST(bdmysql_test, Base) {
	sess.CreateTableIfNotExists(Person().ToOM(), "person", "id,name", "MEMORY");

	Person person1;
	person1.set_id(0);
	person1.set_name("Jonh1");
	person1.set_age(12);
	person1.set_salary(std::nullopt);
	person1.set_born(std::chrono::system_clock::now());
	sess.ReplaceInto("person", person1.ToOM());

	Person person2;
	person2.set_id(1);
	person2.set_name("David");
	person2.set_age(22);
	person2.set_salary(9999);
	person2.set_born(std::chrono::system_clock::now());
	sess.ReplaceInto("person", person2.ToOM());

	Person person3;
	person3.set_id(2);
	person3.set_name("Jack");
	person3.set_age(0);
	person3.set_salary(9999);
	person3.set_born(std::chrono::system_clock::now());
	sess.ReplaceInto("person", person3.ToOM());

	std::vector<std::map<std::string, std::any>> rsom = sess.Select("person", "id>=0 ORDER BY id", Person().ToOM());
	std::vector<Person> rs;
	for (auto& obj : rsom) {
		Person a;
		a = obj;
		rs.push_back(a);
	}

	EXPECT_TRUE(sess.table_exists("bdmysql", "person"));

	EXPECT_TRUE(rs.size() >= 2);
	EXPECT_EQ(rs[0].id(), 0);
	EXPECT_EQ(rs[0].name(), "Jonh1");
	EXPECT_EQ(rs[0].salary(), std::nullopt);
	EXPECT_EQ(std::chrono::duration_cast<std::chrono::seconds>(rs[0].born().time_since_epoch()).count(), std::chrono::duration_cast<std::chrono::seconds>(person1.born().time_since_epoch()).count());
	EXPECT_EQ(rs[1].id(), 1);
	EXPECT_EQ(rs[1].name(), "David");
	rs[1].set_name("David2");
	EXPECT_EQ(rs[1].name(), "David2");
	EXPECT_EQ(rs[1].salary(), 9999);
	EXPECT_EQ(std::chrono::duration_cast<std::chrono::seconds>(rs[1].born().time_since_epoch()).count(), std::chrono::duration_cast<std::chrono::seconds>(person2.born().time_since_epoch()).count());

	EXPECT_EQ(rs[2].name(), "Jack");
}

TEST(bdmysql_test, Where) {
	auto rs = sess.Select("person", "id = 2", Person().ToOM());
	EXPECT_EQ(rs.size(), 1);
	Person p = rs[0];
	EXPECT_EQ(p.id(), 2);
}
