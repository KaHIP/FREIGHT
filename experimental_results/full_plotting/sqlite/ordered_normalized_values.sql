/* List all objective values for a given algorithm normalized by the algorithm with highest value per instance  */
SELECT	row_number() over (
		ORDER BY normalized_objective ASC
		) AS ind,
	normalized_objective AS "generic_label"
FROM (
	SELECT generic_objective / max_generic_objective AS normalized_objective
	FROM	generic_table a
	JOIN	(
		SELECT	graph,
			k, 
			MAX(generic_objective) AS max_generic_objective  
		FROM generic_table
		GROUP BY graph, k
		) c
	ON	a.graph = c.graph AND
		a.k = c.k
	WHERE	algorithm = "generic_algorithm"
	GROUP BY a.graph, a.k
	)
ORDER BY normalized_objective ASC;
