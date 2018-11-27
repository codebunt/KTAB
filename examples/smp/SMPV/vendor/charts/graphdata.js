function GraphData(file, onloaddb) {
    this.dbfile = file;
    this.actorsById = {};
    this.linksByBid = {};
    var __this = this;
    this.onloaddb = onloaddb
    this.init = function () {
        var fr = new FileReader();
        fr.onload = function () {
            var Uints = new Uint8Array(fr.result);
            __this.db = new window.SQL.Database(Uints);
            var maxturn = __this.db.exec("select max(Turn_t) as turns from Bargn");
            __this.turns = maxturn[0].values[0][0];
            __this.initActorData();
            __this.loadTurnData(0);
            __this.onloaddb();

        }
        fr.readAsArrayBuffer(this.dbfile);
    }

    this.init();

    this.clearBargainNodes = function () {
        var clone = {};
        for (var key in __this.actorsById) {
            if (__this.actorsById.hasOwnProperty(key)) {
                if (__this.actorsById[key].type == 'actor') {
                    clone[key] = __this.actorsById[key];
                }
            }
        }
        __this.actorsById = clone;
    }


    this.loadTurnData = function (turn) {
        __this.turn = turn;
        __this.linksByBid = {};
        __this.clearBargainNodes();
        var networkData = __this.db.exec("select B.ScenarioID, B.Turn_t, B.BargnID, B.Init_Act_i, B.Recd_Act_j, AI.Name as Init, AR.Name as Rcvr from Bargn as B inner join " +
            " ActorDescription as AI on B.Init_Act_i = AI.Act_i and B.ScenarioID = AI.ScenarioID inner join " +
            " ActorDescription as AR on B.Recd_Act_j = AR.Act_i and B.ScenarioID = AR.ScenarioID and B.Turn_t = " + turn +
            " order by Turn_t, BargnID");

        networkData[0].values.forEach(link => {
            var from = link[networkData[0].columns.indexOf('Init_Act_i')];
            var to = link[networkData[0].columns.indexOf('Recd_Act_j')];
            var bgn = link[2];
            var id1 = from + " - " + bgn;
            var id2 = bgn + " - " + to;
            var toself = (from == to);
            if(!toself) {
                __this.linksByBid[id1] = {
                    source: from,
                    target: bgn,
                    strength: 2.0,
                    dist: 50,
                    type: "tob",
                    toself: toself,
                    accepted: false
                };
                __this.linksByBid[id2] = {
                    source: bgn,
                    target: to,
                    strength: 2.0,
                    dist: 50,
                    type: "fromb",
                    toself: toself,
                    accepted: false
                };
                __this.actorsById[bgn] = {
                    id: bgn,
                    label: '',
                    type: 'bargain'
                };
            }
        });

        var acceptedBgn = __this.db.exec("select B.*, AI.Name as Init, AR.Name as Rcvr" +
            " from ( " +
            " select ScenarioId, Turn_t, BargnId, Init_Act_i, Recd_Act_j ,'Init' as Q " +
            "from Bargn " +
            "where Init_Seld = 1 and Turn_t = " + turn +
            " union " +
            " select ScenarioId, Turn_t, BargnId, Init_Act_i, Recd_Act_j ,'Rcvr' " +
            " from Bargn " +
            " where Recd_Seld = 1 and Turn_t = " + turn +
            ") as B inner join " +
            " ActorDescription as AI on B.Init_Act_i = AI.Act_i and B.ScenarioID = AI.ScenarioID inner join " +
            " ActorDescription as AR on B.Recd_Act_j = AR.Act_i and B.ScenarioID = AR.ScenarioID");
        acceptedBgn[0].values.forEach(bargn => {
            var from = bargn[acceptedBgn[0].columns.indexOf('Init_Act_i')];
            var to = bargn[acceptedBgn[0].columns.indexOf('Recd_Act_j')];
            var bgn = bargn[acceptedBgn[0].columns.indexOf('BargnId')];
            var q = bargn[acceptedBgn[0].columns.indexOf('Q')];
            var id1 = from + " - " + bgn;
            var id2 = bgn + " - " + to;
            if (__this.linksByBid[id1]) {
                __this.linksByBid[id1].accepted = __this.linksByBid[id2].toself || __this.linksByBid[id1].accepted || (q == 'Init' ? true : false);
            }
            if (__this.linksByBid[id2]) {
                __this.linksByBid[id2].accepted = __this.linksByBid[id2].toself || __this.linksByBid[id2].accepted || (q == 'Rcvr' ? true : false);
            }
        });
        __this.loadEfPower();
        __this.loadPosition();
        if (__this.gren) {
            __this.gren.destroy();
        }
        __this.gren = new GraphRenderer(__this.getActors(), __this.getLinks());
        if (++__this.turn <= __this.turns) {
            setTimeout(() => {
                //__this.loadTurnData(__this.turn);
            }, 5000);
        }
    }

    this.loadEfPower = function () {
        var effPowerData = __this.db.exec("select distinct d.scenarioid, c.Act_i , d.Cap * b.Sal as fpower, b.Dim_k  from SpatialCapability d, ActorDescription c, SpatialSalience b " +
            "where c.Act_i = b.Act_i and b.Act_i = d.Act_i  and d.ScenarioId = b.ScenarioId and d.ScenarioId = c.ScenarioId and b.Turn_t = " + __this.turn + " order by b.Dim_k");
        effPowerData[0].values.forEach(bargn => {
            var aid = bargn[effPowerData[0].columns.indexOf('Act_i')];
            var fp = bargn[effPowerData[0].columns.indexOf('fpower')];
            __this.actorsById[aid].fpower = fp;
        });
    }

    this.loadPosition = function () {
        var actorsData = __this.db.exec("select distinct c.Act_i, a.Turn_t, a.Dim_k , a.Pos_Coord , b.Sal, b.scenarioid, d.Scenario from actordescription c, VectorPosition a," +
            " SpatialSalience b, scenarioDesc d where c.Act_i = a.Act_i and c.Act_i = b.Act_i and a.ScenarioId = b.ScenarioId and c.ScenarioId = b.ScenarioId and d.ScenarioId = b.ScenarioId and a.Act_i = b.Act_i and " +
            "  a.Dim_k = b.Dim_k and a.Turn_t = " + __this.turn + " order by a.Dim_k");
        actorsData[0].values.forEach(bargn => {
            var aid = bargn[actorsData[0].columns.indexOf('Act_i')];
            var pos = bargn[actorsData[0].columns.indexOf('Pos_Coord')];
            __this.actorsById[aid].position = pos;
        });
    }

    this.initActorData = function () {
        var actorsData = __this.db.exec("select Act_i as id, Name as name, Desc as description from ActorDescription");
        actorsData[0].values.forEach(actor => {
            var aid = actor[actorsData[0].columns.indexOf('id')];
            __this.actorsById[aid] = {
                id: aid,
                label: actor[actorsData[0].columns.indexOf('name')],
                type: 'actor'
            };
        });
    }

    this.getActors = function () {
        var ret = [];
        for (var key in __this.actorsById) {
            if (__this.actorsById.hasOwnProperty(key)) {
                ret.push(__this.actorsById[key]);
            }
        }
        console.log(ret);
        return ret;
    }

    this.getLinks = function () {
        var ret = [];
        for (var key in __this.linksByBid) {
            if (__this.linksByBid.hasOwnProperty(key)) {
                ret.push(__this.linksByBid[key]);
            }
        }
        return ret;
    }

}

function GraphRenderer(actors, links) {
    var __this = this;
    this.nodes = actors;
    this.links = links;
    this.destroy = function () {
        d3.selectAll("svg > *").remove();
    };
    this.init = function () {
        var width = window.innerWidth
        var height = window.innerHeight
        var svg = d3.select('svg')
        svg.attr('width', width).attr('height', height);
        var marker = svg.append('defs').selectAll("marker")
            .data(['fromb', 'tob'])
            .enter().append("marker")
            .attr("id", function (d) { return d; })
            .attr("viewBox", '-0 -5 10 10')
            .attr("refX", function (d) { return d === 'fromb' ? 5 : 5; })
            .attr("refY", 0)
            .attr("orient", "auto")
            .attr("markerWidth", function (d) { return d === 'fromb' ? 8 : 8; })
            .attr("markerHeight", function (d) { return d === 'fromb' ? 8 : 8; })
            .attr("xoverflow", 'visible')
            .append('svg:path')
            .attr('d', 'M 0,-5 L 5 ,0 L 0,5')
            .attr('fill', 'red')
            .style('stroke', 'none');

        // simulation setup with all forces
        var linkForce = d3
            .forceLink()
            .id(function (link) { return link.id })
            .distance(function (d) {
                return d.dist ? d.dist : 50;
            })
            .strength(function (link) { return link.strength });

        var dragDrop = d3.drag().on('start', function (node) {
            node.fx = node.x
            node.fy = node.y
        }).on('drag', function (node) {
            __this.simulation.alphaTarget(0.7).restart()
            node.fx = d3.event.x
            node.fy = d3.event.y
        }).on('end', function (node) {
            if (!d3.event.active) {
                __this.simulation.alphaTarget(0)
            }
            node.fx = null
            node.fy = null
        })

        __this.simulation = d3
            .forceSimulation()
            .force('link', linkForce)
            .force('charge', d3.forceManyBody().strength(-120))
            .force('center', d3.forceCenter(width / 2, height / 2));
        var linkElements = svg.append("g")
            .attr("class", "links")
            .selectAll("path")
            .data(__this.links)
            .enter().append("path")
            .attr("class", function (d) { return "link " + d.type + " " + (d.accepted ? "" : "rejectb"); })
            .attr("stroke-width", "1.5px")
            .attr("stroke", "rgba(50, 50, 50, 0.2)")
            .attr('marker-end', function (d) { return d.type == 'fromb' ? "url(#" + d.type + ")" : "" })
        var nodeElements = svg.append("g")
            .attr("class", "nodes")
            .selectAll("circle")
            .data(__this.nodes)
            .enter().append("circle")
            .attr("r", __this.getNodeRadius)
            .attr("class", function (n) { return (n.type == 'actor') ? "actor" : "bargain" })
            .attr("fill", __this.getNodeColor)
            .call(dragDrop)

        var textElements = svg.append("g")
            .attr("class", "texts")
            .selectAll("text")
            .data(__this.nodes)
            .enter().append("text")
            .text(function (node) { return node.label })
            .attr("font-size", 15)
            .attr("dx", 15)
            .attr("dy", 4)
        __this.simulation.nodes(__this.nodes).on('tick', () => {
            nodeElements
                .attr('cx', function (node) { return node.x })
                .attr('cy', function (node) { return node.y })
            textElements
                .attr('x', function (node) { return node.x })
                .attr('y', function (node) { return node.y })
            linkElements
                .attr("d", __this.linkArc);

        })
        __this.simulation.force("link").links(__this.links)

    }

    this.getNodeColor = function (node) {
        return node.type == 'actor' ? __this.getColor(node.fpower / 50) : 'gray'
    }

    this.getColor = function (value) {
        var hue = ((1 - value) * 120).toString(10);
        return ["hsl(", hue, ",100%,50%)"].join("");
    }


    this.getNodeRadius = function (node) {
        if (node.type === 'actor') {
            node.radius = (node.fpower / 2) + ((Math.random() * 100) % 10);
        } else {
            node.radius = 3;
        }
        return node.radius;
    }

    this.linkArc = function (d) {
        var dx = d.target.x - d.source.x,
            dy = d.target.y - d.source.y,
            dr = Math.sqrt(dx * dx + dy * dy);
        if (!d.toself) {
            return "M" + d.source.x + "," + d.source.y + "L" + d.target.x + "," + d.target.y;
        }
        return "M" + d.source.x + "," + d.source.y + "A" + dr + "," + dr + " 0 0,1 " + d.target.x + "," + d.target.y;
    }

    this.linkArc = function (d) {
        var dx = d.target.x - d.source.x,
            dy = d.target.y - d.source.y,
            dr = Math.sqrt(dx * dx + dy * dy);
        diffX = d.target.x - d.source.x;
        diffY = d.target.y - d.source.y;

        // Length of path from center of source node to center of target node
        pathLength = Math.sqrt((diffX * diffX) + (diffY * diffY));

        // x and y distances from center to outside edge of target node
        offsetX = (diffX * d.target.radius) / pathLength;
        offsetY = (diffY * d.target.radius) / pathLength;

        if (!d.toself) {
            return "M" + d.source.x + "," + d.source.y + "L" + (d.target.x - offsetX) + "," + (d.target.y - offsetY);
        }
        return "M" + (d.source.x) + "," + (d.source.y) + "A" + (dr) + "," + (dr) + " 0 0,1 " + (d.target.x - offsetX) + "," + (d.target.y - offsetY);
    }


    this.transform = function (d) {
        return "translate(" + d.x + "," + d.y + ")";
    }

    this.init();
}
var gd;
function plotGraph() {
    var files = document.getElementById("uploadInput").files;
    var file = files[0];
    if (gd) gd.gren.destroy();
    gd = new GraphData(file, function () {
        var range = document.getElementById('turns');
        if (range.noUiSlider) {
            range.noUiSlider.updateOptions({
                start: 0,
                range: {
                    min: 0,
                    max: gd.turns
                },
                step: 1,
                connect: true,
                behaviour: 'tap-drag',
                tooltips: false,
                pips: {
                    mode: 'steps',
                    stepped: true,
                    density: 4
                }
            });
        }
        else {
            noUiSlider.create(range, {
                start: 0,
                range: {
                    min: 0,
                    max: gd.turns
                },

                step: 1,

                connect: true,

                // Move handle on tap, bars are draggable
                behaviour: 'tap-drag',
                tooltips: false,

                // Show a scale with the slider
                pips: {
                    mode: 'steps',
                    stepped: true,
                    density: 4
                }
            });
        }

        range.noUiSlider.on('update', function (values, handle) {
            var value = values[handle];
            gd.loadTurnData(value);
        });

    });

}