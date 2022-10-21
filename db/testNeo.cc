#include <neo4j-client.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    neo4j_client_init();

    /* use NEO4J_INSECURE when connecting to disable TLS */
    neo4j_connection_t *connection =
            neo4j_connect("neo4j://neo4j:hello4156@localhost:7687", NULL, NEO4J_INSECURE);
    if (connection == NULL)
    {
        neo4j_perror(stderr, errno, "Connection failed");
        return EXIT_FAILURE;
    }

    neo4j_result_stream_t *results =
            neo4j_run(connection, "MATCH (n:User) RETURN n", neo4j_null);
    //neo4j_result_stream_t *results = neo4j_run(connection, "MATCH (a:User {email: 'c@163.com'}), (b:TaskList {name: 'hello'}) MERGE (a)-[r:Owns]->(b)", neo4j_null);
    if (results == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to run statement");
        return EXIT_FAILURE;
    }

    if (neo4j_check_failure(results))
    {
        printf("%s\n", neo4j_error_message(results));
        printf("%s\n", neo4j_error_code(results));
        neo4j_perror(stderr, errno, "Statement failed");
        return EXIT_FAILURE;
    }

    neo4j_result_t *result = neo4j_fetch_next(results);
    if (result == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to fetch result");
        return EXIT_FAILURE;
    }

    neo4j_value_t value = neo4j_result_field(result, 0);
    value = neo4j_node_properties(value);
    value = neo4j_map_get(value, "abc");
    char buf[1024];
    printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));

    result = neo4j_fetch_next(results);
    if (result == NULL)
    {
        neo4j_perror(stderr, errno, "Failed to fetch result");
        return EXIT_FAILURE;
    }

    value = neo4j_result_field(result, 0);
    value = neo4j_node_properties(value);
    value = neo4j_map_get(value, "email");
    printf("%s\n", neo4j_tostring(value, buf, sizeof(buf)));

    neo4j_close_results(results);
    neo4j_close(connection);
    neo4j_client_cleanup();
    return EXIT_SUCCESS;
}