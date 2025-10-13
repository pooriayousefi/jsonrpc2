/*
 * JSON-RPC 2.0 Database/CRUD Service Example
 *
 * Demonstrates a real-world CRUD (Create, Read, Update, Delete) service
 * using JSON-RPC 2.0. Simulates a simple in-memory user database.
 *
 * Features:
 * - Create, Read, Update, Delete operations
 * - Query and filtering
 * - Batch operations
 * - Transaction-like operations
 * - Error handling for invalid operations
 */

#include "../include/jsonrpc.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace pooriayousefi;
using json = nlohmann::json;

struct User
{
    int id;
    std::string name;
    std::string email;
    int age;
    bool active;

    json to_json() const
    {
        return json{{"id", id}, {"name", name}, {"email", email}, {"age", age}, {"active", active}};
    }

    static User from_json(const json &j)
    {
        User u;
        u.id = j.value("id", 0);
        u.name = j.value("name", "");
        u.email = j.value("email", "");
        u.age = j.value("age", 0);
        u.active = j.value("active", true);
        return u;
    }
};

class DatabaseService
{
  private:
    std::map<int, User> users_;
    int next_id_;
    dispatcher dispatcher_;

    // Validation helpers
    void validate_user_data(const json &data, bool require_id = false)
    {
        if (!data.is_object())
        {
            throw rpc_exception(
                error{-32602, "Invalid params", json{{"detail", "User data must be an object"}}});
        }
        if (require_id && !data.contains("id"))
        {
            throw rpc_exception(
                error{-32602, "Invalid params", json{{"detail", "User ID is required"}}});
        }
        if (!require_id && !data.contains("name"))
        {
            throw rpc_exception(
                error{-32602, "Invalid params", json{{"detail", "User name is required"}}});
        }
        if (!require_id && !data.contains("email"))
        {
            throw rpc_exception(
                error{-32602, "Invalid params", json{{"detail", "User email is required"}}});
        }
    }

  public:
    DatabaseService() : next_id_(1)
    {
        // CREATE
        dispatcher_.add("user.create",
                        [this](const json &params) -> json
                        {
                            validate_user_data(params, false);

                            User user = User::from_json(params);
                            user.id = next_id_++;
                            users_[user.id] = user;

                            return json{
                                {"success", true}, {"id", user.id}, {"user", user.to_json()}};
                        });

        // READ - Get single user
        dispatcher_.add(
            "user.get",
            [this](const json &params) -> json
            {
                if (!params.contains("id"))
                {
                    throw rpc_exception(
                        error{-32602, "Invalid params", json{{"detail", "User ID is required"}}});
                }
                int id = params["id"].get<int>();
                auto it = users_.find(id);
                if (it == users_.end())
                {
                    throw rpc_exception(error{-32001, "User not found", json{{"id", id}}});
                }
                return it->second.to_json();
            });

        // READ - List all users
        dispatcher_.add("user.list",
                        [this](const json &params) -> json
                        {
                            (void)params;
                            json users_array = json::array();
                            for (const auto &[id, user] : users_)
                            {
                                users_array.push_back(user.to_json());
                            }
                            return json{{"users", users_array}, {"count", users_array.size()}};
                        });

        // READ - Query users
        dispatcher_.add("user.query",
                        [this](const json &params) -> json
                        {
                            json result = json::array();

                            // Filter by active status
                            bool filter_active = params.contains("active");
                            bool active_value = filter_active ? params["active"].get<bool>() : true;

                            // Filter by minimum age
                            int min_age = params.value("min_age", 0);

                            // Filter by name pattern
                            std::string name_pattern = params.value("name_pattern", "");

                            for (const auto &[id, user] : users_)
                            {
                                bool matches = true;

                                if (filter_active && user.active != active_value)
                                    matches = false;

                                if (user.age < min_age)
                                    matches = false;

                                if (!name_pattern.empty() &&
                                    user.name.find(name_pattern) == std::string::npos)
                                    matches = false;

                                if (matches)
                                {
                                    result.push_back(user.to_json());
                                }
                            }

                            return json{{"users", result}, {"count", result.size()}};
                        });

        // UPDATE
        dispatcher_.add("user.update",
                        [this](const json &params) -> json
                        {
                            validate_user_data(params, true);

                            int id = params["id"].get<int>();
                            auto it = users_.find(id);
                            if (it == users_.end())
                            {
                                throw rpc_exception(
                                    error{-32001, "User not found", json{{"id", id}}});
                            }

                            // Update fields if provided
                            if (params.contains("name"))
                                it->second.name = params["name"].get<std::string>();
                            if (params.contains("email"))
                                it->second.email = params["email"].get<std::string>();
                            if (params.contains("age"))
                                it->second.age = params["age"].get<int>();
                            if (params.contains("active"))
                                it->second.active = params["active"].get<bool>();

                            return json{{"success", true}, {"user", it->second.to_json()}};
                        });

        // DELETE
        dispatcher_.add(
            "user.delete",
            [this](const json &params) -> json
            {
                if (!params.contains("id"))
                {
                    throw rpc_exception(
                        error{-32602, "Invalid params", json{{"detail", "User ID is required"}}});
                }
                int id = params["id"].get<int>();
                auto it = users_.find(id);
                if (it == users_.end())
                {
                    throw rpc_exception(error{-32001, "User not found", json{{"id", id}}});
                }
                json deleted_user = it->second.to_json();
                users_.erase(it);
                return json{{"success", true}, {"deleted", deleted_user}};
            });

        // Batch CREATE
        dispatcher_.add(
            "user.batch_create",
            [this](const json &params) -> json
            {
                if (!params.contains("users") || !params["users"].is_array())
                {
                    throw rpc_exception(error{-32602, "Invalid params",
                                              json{{"detail", "users array is required"}}});
                }

                json results = json::array();
                for (const auto &user_data : params["users"])
                {
                    try
                    {
                        validate_user_data(user_data, false);
                        User user = User::from_json(user_data);
                        user.id = next_id_++;
                        users_[user.id] = user;
                        results.push_back(json{{"success", true}, {"id", user.id}});
                    }
                    catch (const rpc_exception &e)
                    {
                        results.push_back(json{{"success", false}, {"error", e.err.message}});
                    }
                }
                return json{{"results", results}};
            });

        // Statistics
        dispatcher_.add("user.stats",
                        [this](const json &params) -> json
                        {
                            (void)params;
                            int total = users_.size();
                            int active = 0;
                            int inactive = 0;
                            double avg_age = 0.0;

                            for (const auto &[id, user] : users_)
                            {
                                if (user.active)
                                    active++;
                                else
                                    inactive++;
                                avg_age += user.age;
                            }

                            if (total > 0)
                                avg_age /= total;

                            return json{{"total", total},
                                        {"active", active},
                                        {"inactive", inactive},
                                        {"average_age", avg_age}};
                        });

        // Clear all users
        dispatcher_.add("user.clear",
                        [this](const json &params) -> json
                        {
                            (void)params;
                            size_t count = users_.size();
                            users_.clear();
                            next_id_ = 1;
                            return json{{"success", true}, {"deleted_count", count}};
                        });
    }

    json handle_request(const json &request) { return *dispatcher_.handle(request); }

    size_t user_count() const { return users_.size(); }
};

// ============================================================================
// Demo and Test Functions
// ============================================================================

void run_database_demo(DatabaseService &db)
{
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "Database/CRUD Service Demo\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    // CREATE operations
    std::cout << "1. CREATE Operations:\n";
    {
        json req = make_request(
            1, "user.create",
            json{{"name", "Alice Johnson"}, {"email", "alice@example.com"}, {"age", 28}});
        std::cout << "  Creating user: Alice Johnson\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    {
        json req =
            make_request(2, "user.create",
                         json{{"name", "Bob Smith"}, {"email", "bob@example.com"}, {"age", 35}});
        std::cout << "  Creating user: Bob Smith\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    {
        json req = make_request(
            3, "user.create",
            json{{"name", "Carol White"}, {"email", "carol@example.com"}, {"age", 42}});
        std::cout << "  Creating user: Carol White\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // READ operations
    std::cout << "2. READ Operations:\n";
    {
        json req = make_request(4, "user.get", json{{"id", 1}});
        std::cout << "  Getting user with ID 1:\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    {
        json req = make_request(5, "user.list", json::object());
        std::cout << "  Listing all users:\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // UPDATE operations
    std::cout << "3. UPDATE Operations:\n";
    {
        json req = make_request(6, "user.update", json{{"id", 1}, {"age", 29}, {"active", true}});
        std::cout << "  Updating user ID 1 (age to 29):\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // QUERY operations
    std::cout << "4. QUERY Operations:\n";
    {
        json req = make_request(7, "user.query", json{{"min_age", 30}});
        std::cout << "  Querying users with age >= 30:\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // Statistics
    std::cout << "5. STATISTICS:\n";
    {
        json req = make_request(8, "user.stats", json::object());
        std::cout << "  Getting database statistics:\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // Batch CREATE
    std::cout << "6. BATCH CREATE:\n";
    {
        json users_batch = json::array();
        users_batch.push_back(
            json{{"name", "David Lee"}, {"email", "david@example.com"}, {"age", 25}});
        users_batch.push_back(
            json{{"name", "Eve Davis"}, {"email", "eve@example.com"}, {"age", 31}});

        json req = make_request(9, "user.batch_create", json{{"users", users_batch}});
        std::cout << "  Creating multiple users in batch:\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // DELETE operation
    std::cout << "7. DELETE Operation:\n";
    {
        json req = make_request(10, "user.delete", json{{"id", 2}});
        std::cout << "  Deleting user with ID 2:\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // Final list
    std::cout << "8. FINAL STATE:\n";
    {
        json req = make_request(11, "user.list", json::object());
        std::cout << "  Listing all remaining users:\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }

    // Error handling
    std::cout << "9. ERROR HANDLING:\n";
    {
        json req = make_request(12, "user.get", json{{"id", 999}});
        std::cout << "  Attempting to get non-existent user (ID 999):\n";
        json resp = db.handle_request(req);
        std::cout << "  Response: " << resp.dump(2) << "\n\n";
    }
}

// ============================================================================
// Main Entry Point
// ============================================================================

int run_database_service()
{
    try
    {
        DatabaseService db;
        run_database_demo(db);

        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Database service completed successfully!\n";
        std::cout << "Final user count: " << db.user_count() << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in database service: " << e.what() << std::endl;
        return 1;
    }
}
