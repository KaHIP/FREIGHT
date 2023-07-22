/* Obtain statistic metric for each algorithm  */
SELECT	a.algorithm, 
	ROUND(EXP(AVG(LN(generic_objective))),3) AS GeoMean_generic_objective 
FROM generic_table a
JOIN	(
	SELECT	algorithm,
		AVG(generic_objective) AS mean_value
	FROM generic_table
	GROUP BY algorithm
	) b
ON a.algorithm = b.algorithm
JOIN	(
	SELECT	x.algorithm,
		AVG(x.generic_objective) AS median_objective
	FROM	(
		SELECT	row_number() over (
				ORDER BY algorithm, generic_objective ASC
			) AS ind,
			*
		FROM generic_table
		) x
	JOIN	(
		SELECT	algorithm,
			( MAX(ind)+MIN(ind) ) / 2 AS median_ind
		FROM	(
			SELECT	row_number() over (
					ORDER BY algorithm, generic_objective ASC
				) AS ind,
				*
			FROM generic_table
			)
		GROUP BY algorithm
		) y
	ON	x.algorithm = y.algorithm
	WHERE x.ind = y.median_ind
	GROUP BY x.algorithm
	) c
ON b.algorithm = c.algorithm
GROUP BY a.algorithm;
