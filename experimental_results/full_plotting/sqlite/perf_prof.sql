/* Obtain performance profile for a given algorithm  */
SELECT	row_number() over (
		ORDER BY ratio_to_best ASC
		) / CAST(total AS REAL) AS ind,
	ratio_to_best AS "generic_label"
FROM	(
	SELECT	CASE WHEN c.generic_objective = 0 THEN
			CASE WHEN a.generic_objective = 0 THEN
				1.0 
			ELSE
				1000.0
			END
		ELSE
			(a.generic_objective)/c.generic_objective 
		END AS ratio_to_best 
	FROM generic_table a 
	INNER JOIN (	
		SELECT b.graph, 
			b.seed, 
			b.k, 
			MIN(b.generic_objective) AS "generic_objective"
		FROM generic_table b 
		GROUP BY b.graph, 
			b.seed, 
			b.k
		) c  
	ON	a.k = c.k AND 
		a.graph = c.graph AND 
		a.seed = c.seed 
	WHERE a.algorithm = "generic_algorithm"
	ORDER BY ratio_to_best ASC
	),
	(SELECT COUNT(*) AS total
	FROM generic_table
	WHERE algorithm = "generic_algorithm");
