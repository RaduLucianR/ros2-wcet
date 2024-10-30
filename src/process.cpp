/**
 * This will be a *GENERATED* file.
 * Steps for generation:
 * 
 * Iterate over the .json files that describe Node classes
 *  - start stress program
 *  for each .json file:
 *      - #include its .hpp file
 * 
 *      for each Node Class object in the .json file:
 *          for each callback in the Node Class:
 *              - TODO: somehow generate "worst case" parameters (?????) [this is "only" for subscription callbacks]
 *              - write the node->callback(params) as parameter for the measurement function
 *              - call the measurement template function
 *              - write csv with measurements
 *              - take max of measurements
 */