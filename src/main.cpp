#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <pqxx/pqxx>
#include "crow.h"

struct BookData
{
    std::string title;
    std::string authors;
    float rating;
};


std::unordered_map<std::string, BookData> book_catalog;
std::unordered_map<int, std::vector<std::string>> user_to_books;
std::unordered_map<std::string, std::vector<int>> book_to_users;


class TrieNode
{
public:
    std::unordered_map<char, TrieNode *> children;
    bool isEndOfWord;
    std::vector<std::string> exact_titles; 

    TrieNode() : isEndOfWord(false) {}
};

class Trie
{
private:
    TrieNode *root;

    void dfs(TrieNode *node, std::vector<std::string> &res, int &limit)
    {
        if (!node || limit <= 0)
            return;

        if (node->isEndOfWord)
        {
            for (const auto &t : node->exact_titles)
            {
                res.push_back(t);
                limit--;
                if (limit <= 0)
                    return;
            }
        }

        for (auto &pair : node->children)
        {
            dfs(pair.second, res, limit);
            if (limit <= 0)
                return;
        }
    }

public:
    Trie() { root = new TrieNode(); }

    void insert(const std::string &title)
    {
        TrieNode *curr = root;
        for (char c : title)
        {
            char lc = std::tolower(c); // Make Trie case-insensitive
            if (!curr->children.count(lc))
            {
                curr->children[lc] = new TrieNode();
            }
            curr = curr->children[lc];
        }
        curr->isEndOfWord = true;
        curr->exact_titles.push_back(title); // Save original title
    }

    std::vector<std::string> get_suggestions(const std::string &prefix, int limit = 5)
    {
        TrieNode *curr = root;
        for (char c : prefix)
        {
            char lc = std::tolower(c);
            if (!curr->children.count(lc))
                return {}; // Prefix not found
            curr = curr->children[lc];
        }

        std::vector<std::string> results;
        dfs(curr, results, limit);
        return results;
    }
};


Trie search_trie;

//DATABASE INITIALIZATION


void load_backend_data()
{
    try
    {
        // Connect to Postgres Docker Container
        pqxx::connection C("postgresql://bhaskar:password@db:5432/bookshelf");
        if (!C.is_open())
            return;
        pqxx::work W(C);

        // Fetch Book Metadata & Build Trie
        pqxx::result books_res = W.exec("SELECT isbn, title, authors, rating FROM books");
        for (auto row : books_res)
        {
            std::string isbn = row[0].as<std::string>();
            std::string title = row[1].as<std::string>();

            book_catalog[isbn] = {title, row[2].as<std::string>(), row[3].as<float>(0.0)};

            // Insert title into our Search Trie
            search_trie.insert(title);
        }

        // Fetch Graph Edges for Recommendations
        pqxx::result likes_res = W.exec("SELECT user_id, isbn FROM likes");
        for (auto row : likes_res)
        {
            int u_id = row[0].as<int>();
            std::string isbn = row[1].as<std::string>();
            user_to_books[u_id].push_back(isbn);
            book_to_users[isbn].push_back(u_id);
        }

        std::cout << "SUCCESS: Loaded " << book_catalog.size() << " books into Trie and Graph.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database Error: " << e.what() << std::endl;
    }
}


//GRAPH ALGORITHM

std::vector<std::pair<std::string, int>> get_recommendations(int target_user)
{
    std::unordered_map<std::string, int> book_scores;
    std::unordered_set<std::string> already_read;

    if (user_to_books.find(target_user) == user_to_books.end())
        return {};

    for (const std::string &book : user_to_books[target_user])
        already_read.insert(book);

    for (const std::string &read_book : user_to_books[target_user])
    {
        for (int similar_user : book_to_users[read_book])
        {
            if (similar_user == target_user)
                continue;
            for (const std::string &candidate_book : user_to_books[similar_user])
            {
                if (already_read.find(candidate_book) == already_read.end())
                {
                    book_scores[candidate_book]++;
                }
            }
        }
    }

    std::vector<std::pair<std::string, int>> sorted_recs(book_scores.begin(), book_scores.end());
    std::sort(sorted_recs.begin(), sorted_recs.end(),
              [](const auto &a, const auto &b)
              { return a.second > b.second; });
    return sorted_recs;
}

// REST API ROUTES (CROW FRAMEWORK)


int main()
{
    std::cout << "Starting BookShelf API Server...\n";
    load_backend_data(); 

    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/search")
    ([](const crow::request &req)
     {
        crow::json::wvalue response;
        
        char* query_param = req.url_params.get("q");
        if (!query_param) {
            response["error"] = "Missing search query parameter 'q'";
            return crow::response(400, response);
        }

        std::string query(query_param);
        

        std::vector<std::string> suggestions = search_trie.get_suggestions(query, 5);
        

        std::vector<crow::json::wvalue> json_suggestions;
        for (const std::string& s : suggestions) {
            json_suggestions.push_back(s);
        }
        
        response["query"] = query;
        response["results"] = std::move(json_suggestions);
        
        return crow::response(200, response); });


    CROW_ROUTE(app, "/api/recommend/<int>")
    ([](int user_id)
     {
        auto recommendations = get_recommendations(user_id);
        crow::json::wvalue response;
        
        if (recommendations.empty()) {
            response["status"] = "error";
            response["message"] = "User not found or not enough data.";
            return crow::response(404, response);
        }

        response["status"] = "success";
        std::vector<crow::json::wvalue> rec_list;
        
        int limit = std::min((size_t)5, recommendations.size());
        for (int i = 0; i < limit; ++i) {
            std::string isbn = recommendations[i].first;
            BookData data = book_catalog[isbn];
            
            crow::json::wvalue book_json;
            book_json["title"] = data.title;
            book_json["authors"] = data.authors;
            book_json["graph_score"] = recommendations[i].second;
            rec_list.push_back(std::move(book_json));
        }
        
        response["recommendations"] = std::move(rec_list);
        return crow::response(200, response); });

    app.port(8080).multithreaded().run();
    return 0;
}