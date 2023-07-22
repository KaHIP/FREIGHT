/* List all objective values for a given algorithm  */
SELECT	row_number() over (
		ORDER BY generic_objective ASC
		) AS ind,
	generic_objective AS "generic_label"
FROM	generic_table
WHERE algorithm = "generic_algorithm"
ORDER BY generic_objective ASC;
