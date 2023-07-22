/* For ratio plot */
/* Obtain ratio of arithmetic means of the objective grouped by graph and k */
SELECT	row_number() over (
		ORDER BY ratio ASC
		) AS ind,
	ratio AS "generic_label"
FROM	(
	SELECT	CASE WHEN a.generic_objective = 0 THEN
			CASE WHEN c.generic_objective = 0 THEN
				1.0 
			ELSE
				100.0
			END
		ELSE
			CASE WHEN c.generic_objective = 0 THEN
				0.01
			ELSE
				(1*(c.generic_objective/a.generic_objective))		
			END
		END AS ratio 
	FROM (	SELECT	b.graph,
			b.k, 
			AVG(b.generic_objective) AS generic_objective  
		FROM  generic_table b  
		WHERE b.algorithm="generic_algorithm" 
		GROUP BY b.graph, b.k) a 
	JOIN (	SELECT	b.graph,
			b.k, 
			AVG(b.generic_objective) generic_objective 
		FROM generic_table b 
		WHERE b.algorithm="reference_algorithm" 
		GROUP BY b.graph, b.k) c 
	ON	a.graph = c.graph AND
		a.k = c.k
	);

