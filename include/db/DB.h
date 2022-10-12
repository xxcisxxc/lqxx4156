#include <string>

/**
 * @brief This class connect and interact with Neo4j DB.
 * 
 */
class DB
{
private:
    /* data */

    /* method */
    /**
     * @brief Connect to Neo4j DB.
     * 
     */
    void connectDB();
    /**
     * @brief Disconnect from Neo4j DB.
     * 
     */
    void disconnectDB();
    /**
     * @brief Execute a query.
     * 
     * @param query 
     * @return std::string 
     */
    std::string executeQuery(std::string query);
public:
    /**
     * @brief Construct a new DB object: connect to Neo4j DB.
     * 
     */
    DB(/* args */);
    /**
     * @brief Destroy the DB object: disconnect from Neo4j DB.
     * 
     */
    ~DB();

    /**
     * @brief Create a user node.
     * 
     */
    void createUserNode(/* args */);
    /**
     * @brief Create a task list node.
     * 
     */
    void createTaskListNode(/* args */);
    /**
     * @brief Create a task node.
     * 
     */
    void createTaskNode(/* args */);
    /**
     * @brief Revise a user node.
     * 
     */
    void reviseUserNode(/* args */);
    /**
     * @brief Revise a task list node.
     * 
     */
    void reviseTaskListNode(/* args */);
    /**
     * @brief Revise a task node.
     * 
     */
    void reviseTaskNode(/* args */);
    /**
     * @brief Delete a user node.
     * 
     */
    void deleteUserNode(/* args */);
    /**
     * @brief Delete a task list node.
     * 
     */
    void deleteTaskListNode(/* args */);
    /**
     * @brief Delete a task node.
     * 
     */
    void deleteTaskNode(/* args */);
    /**
     * @brief Get a user node.
     * 
     */
    void getUserNode(/* args */);
    /**
     * @brief Get a task list node.
     * 
     */
    void getTaskListNode(/* args */);
    /**
     * @brief Get a task node.
     * 
     */
    void getTaskNode(/* args */);
};

DB::DB(/* args */)
{
}

DB::~DB()
{
}

/*
unit test:
- each method (mock database?)
*/