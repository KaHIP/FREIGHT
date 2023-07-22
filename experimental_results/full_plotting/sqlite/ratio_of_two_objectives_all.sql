/* List the ratio of two given objectives from two different tables for all instances */
/* SELECT	row_number() over ( */
/* 		ORDER BY ratio ASC */
/* 		) AS ind, */
SELECT	1000000000 * a.generic_objective1 / b.generic_objective2 AS "generic_label"
FROM	generic_table1 a
JOIN	generic_table2 b
ON	a.k = b.k AND
	a.seed = b.seed AND
	a.graph = b.graph AND
	a.algorithm = b.algorithm AND
	a.algorithm = "generic_algorithm"
WHERE	b.algorithm = "generic_algorithm" 
ORDER BY a.generic_objective1/b.generic_objective2 ASC;
