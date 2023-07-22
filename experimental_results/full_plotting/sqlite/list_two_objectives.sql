/* List two given objectives for all instances for a given algorithm  */
SELECT	generic_objective2 AS ind,
	generic_objective1 AS "generic_label"
FROM	generic_table
WHERE	algorithm = "generic_algorithm" AND
	graph = "generic_graph"
ORDER BY generic_objective2, generic_objective1 ASC;
