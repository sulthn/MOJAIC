import {
    temperature
} from './index.js';


export function chart(delay) {
    // Declare the chart dimensions and margins.
    const width = 928;
    const height = 500;
    const marginTop = 20;
    const marginRight = 30;
    const marginBottom = 30;
    const marginLeft = 40;

    // Declare the x (horizontal position) scale.
    const x = d3.scaleLinear()
        .domain(d3.extent(temperature, d => d.time))
        .nice()
        .range([marginLeft, width - marginRight]);

    // Declare the y (vertical position) scale.
    const y = d3.scaleLinear()
        .domain(d3.extent(temperature, d => d.temp))
        .nice()
        .range([height - marginBottom, marginTop]);

    // Declare the line generator.
    /*const line = d3.line()
        .x(d => x(d.time))
        .y(d => y(d.temp))
        .curve(d3.curveLinear);*/

    // Create the SVG container.
    const svg = d3.create("svg")
        .attr("width", width)
        .attr("height", height)
        .attr("viewBox", [0, 0, width, height])
        .attr("style", "max-width: 100%; height: auto; height: intrinsic;");

    // Add the x-axis.
    svg.append("g")
        .attr("transform", `translate(0,${height - marginBottom})`)
        .call(d3.axisBottom(x).ticks(width / 80).tickSizeOuter(0));

    // Add the y-axis, remove the domain line, add grid lines and a label.
    svg.append("g")
        .attr("transform", `translate(${marginLeft},0)`)
        .call(d3.axisLeft(y).ticks(height / 40))
        .call(g => g.select(".domain").remove())
        .call(g => g.selectAll(".tick line").clone()
            .attr("x2", width - marginLeft - marginRight)
            .attr("stroke-opacity", 0.1))

    // Append a path for the line.
    /*svg.append("path")
        .datum(temperature)
        .attr("fill", "none")
        .attr("stroke", "steelblue")
        .attr("stroke-width", 1.5)
        .attr("d", line);*/

    svg.selectAll("circle")
        .data(temperature)
        .enter()
        .append("circle")
        .attr("class", "point")
        .attr("cx", d => x(d.time))
        .attr("cy", d => y(d.temp))
        .attr("r", 5);

    const clean = temperature
        .map(d => ({
            x: Number(d.time),
            y: Number(d.temp)
        }))
        .filter(d => Number.isFinite(d.x) && Number.isFinite(d.y));

    const n = clean.length;

    const sumX = d3.sum(clean, d => d.x);
    const sumY = d3.sum(clean, d => d.y);
    const sumXY = d3.sum(clean, d => d.x * d.y);
    const sumXX = d3.sum(clean, d => d.x * d.x);

    const denom = n * sumXX - sumX * sumX;

    if (denom !== 0) {
        const slope = (n * sumXY - sumX * sumY) / denom;
        const intercept = (sumY - slope * sumX) / n;

        const xMin = d3.min(clean, d => d.x);
        const xMax = d3.max(clean, d => d.x);

        const regressionData = [
            { x: xMin, y: slope * xMin + intercept },
            { x: xMax, y: slope * xMax + intercept }
        ];

        const line = d3.line()
            .x(d => x(d.x))
            .y(d => y(d.y));

        svg.append("path")
            .datum(regressionData)
            .attr("class", "regression")
            .attr("fill", "none")
            .attr("stroke", "steelblue")
            .attr("stroke-width", 1.5)
            .attr("d", line);
    } else {
        console.log("Cannot compute regression: all x values are identical");
    }

    return svg.node();
}

