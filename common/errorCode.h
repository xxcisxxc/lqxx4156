#pragma once
/**
 * @brief return code
 *
 */
enum returnCode {
  SUCCESS,
  ERR_UNKNOWN,  // Unknown error
  ERR_KEY,      // No primary key
  ERR_RFIELD,   // No required field
  ERR_NO_NODE,  // No such node
  ERR_DUP_NODE, // Duplicate node
  ERR_ACCESS,   // Access denied
  ERR_FORMAT,   // Content format error
  ERR_REVISE,   // Field not revisible
};