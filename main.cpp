#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <limits.h>
#include <chrono>
#include <thread>

struct Edge {
    int src, dest, weight;
};

class BellmanFordSimulator {
private:
    sf::RenderWindow window;
    sf::Font font;
    std::vector<Edge> edges;
    std::vector<int> dist;
    int V, E;
    int currentLine;
    
    int currentI;
    int currentJ;
    int currentU;
    int currentV;
    int currentWeight;
    bool isRelaxed;
float graphStartX = 200; // Move graph to the right
float graphStartY = 20; // Move graph lower to avoid overlap with code

    
    // Simulation control
    bool isPlaying;
    float simulationSpeed;  // Steps per second
    sf::Clock stepClock;
    
    // Control buttons
    sf::RectangleShape playPauseButton;
    sf::RectangleShape speedUpButton;
    sf::RectangleShape slowDownButton;
    sf::Text playPauseText;
    sf::Text speedText;
    sf::Text debugInfo;
    
    std::vector<std::string> codeLines;
    
    // Vertex positions for better graph layout
    std::vector<sf::Vector2f> vertexPositions;
    
    void initializeVertexPositions() {
        vertexPositions.resize(V);
        float centerX = 600;
        float centerY = 300;
        float radius = 200;
        
        // Calculate golden angle for more even distribution
        float goldenAngle = 2 * M_PI * 0.618034;
        
        for (int i = 0; i < V; i++) {
            float angle = i * goldenAngle;
            vertexPositions[i] = sf::Vector2f(
                centerX + radius * cos(angle),
                centerY + radius * sin(angle)
            );
        }
    }

    void initializeCode() {
        codeLines = {
            "void BellmanFord(struct Edge edges[], int V, int E, int src) {",
            "    int dist[V];",
            "    // Step 1: Initialize distances from src to all other vertices as INFINITE",
            "    for (int i = 0; i < V; i++) {",
            "        dist[i] = INT_MAX;",
            "    }",
            "    dist[src] = 0;",
            "    // Step 2: Relax all edges |V| - 1 times",
            "    for (int i = 1; i <= V - 1; i++) {",
            "        for (int j = 0; j < E; j++) {",
            "            int u = edges[j].src;",
            "            int v = edges[j].dest;",
            "            int weight = edges[j].weight;",
            "            if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {",
            "                dist[v] = dist[u] + weight;",
            "            }",
            "        }",
            "    }",
            "    // Step 3: Check for negative-weight cycles",
            "    for (int i = 0; i < E; i++) {",
            "        int u = edges[i].src;",
            "        int v = edges[i].dest;",
            "        int weight = edges[i].weight;",
            "        if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {",
            "            printf(\"Graph contains negative weight cycle\");",
            "            return;",
            "        }",
            "    }"
        };
    }

    void drawArray() {
        float startX = 50;
        float startY = 50;
        float cellWidth = 60;
        float cellHeight = 40;

        // Draw array label with nice background
        sf::RectangleShape labelBg(sf::Vector2f(200, 30));
        labelBg.setPosition(startX - 10, startY - 60);
        labelBg.setFillColor(sf::Color(230, 230, 230));
        labelBg.setOutlineColor(sf::Color::Black);
        labelBg.setOutlineThickness(1);
        window.draw(labelBg);

        sf::Text label;
        label.setFont(font);
        label.setString("Distance Array:");
        label.setCharacterSize(20);
        label.setFillColor(sf::Color::Black);
        label.setPosition(startX, startY - 55);
        window.draw(label);

        // Draw array with animations
        for (int i = 0; i < V; i++) {
            // Draw index
            sf::Text indexText;
            indexText.setFont(font);
            indexText.setString(std::to_string(i));
            indexText.setCharacterSize(20);
            indexText.setFillColor(sf::Color::Black);
            indexText.setPosition(startX + i * cellWidth + cellWidth/3, startY - 30);
            window.draw(indexText);

            // Draw cell with animation effects
            sf::RectangleShape cell(sf::Vector2f(cellWidth, cellHeight));
            cell.setPosition(startX + i * cellWidth, startY);
            
            if (i == currentU || i == currentV) {
                // Highlight active cells with gradient
                sf::Color highlightColor = sf::Color(255, 255, 200);
                cell.setFillColor(highlightColor);
            } else {
                cell.setFillColor(sf::Color::White);
            }
            
            cell.setOutlineColor(sf::Color::Black);
            cell.setOutlineThickness(2);
            window.draw(cell);

            // Draw value with animation for changes
            sf::Text valueText;
            valueText.setFont(font);
            valueText.setString(dist[i] == INT_MAX ? "∞" : std::to_string(dist[i]));
            valueText.setCharacterSize(20);
            valueText.setFillColor(sf::Color::Black);
            
            // Center the text in the cell
            sf::FloatRect textBounds = valueText.getLocalBounds();
            valueText.setPosition(
                startX + i * cellWidth + (cellWidth - textBounds.width) / 2,
                startY + (cellHeight - textBounds.height) / 2
            );
            
            window.draw(valueText);
        }
    }

void drawGraph() {
    // Draw edges with curved paths and animations
    for (int i = 0; i < E; i++) {
        sf::Vector2f start = vertexPositions[edges[i].src];
        sf::Vector2f end = vertexPositions[edges[i].dest];
        
        // Offset the graph elements by the graph starting position
        start += sf::Vector2f(graphStartX, graphStartY);
        end += sf::Vector2f(graphStartX, graphStartY);
        
        // Calculate control point for curved edge
        sf::Vector2f mid = (start + end) / 2.0f;
        sf::Vector2f normal = sf::Vector2f(-(end.y - start.y), end.x - start.x);
        float normalLength = sqrt(normal.x * normal.x + normal.y * normal.y);
        normal = normal / normalLength * 30.0f;
        
        // Draw curved edge using multiple lines
        const int segments = 20;
        std::vector<sf::Vertex> curve(segments + 1);
        
        sf::Color edgeColor = sf::Color(100, 100, 100);
        if (i == currentJ) {
            edgeColor = sf::Color::Red;
        }
        else if (isRelaxed && i == currentJ - 1) {
            edgeColor = sf::Color::Green;
        }
        
        for (int j = 0; j <= segments; ++j) {
            float t = j / static_cast<float>(segments);
            float u = 1 - t;
            sf::Vector2f pos = u * u * start + 2 * u * t * (mid + normal) + t * t * end;
            curve[j] = sf::Vertex(pos, edgeColor);
        }
        
        window.draw(&curve[0], curve.size(), sf::LineStrip);

        // Draw weight
        sf::Text weightText;
        weightText.setFont(font);
        weightText.setString(std::to_string(edges[i].weight));
        weightText.setCharacterSize(20);
        weightText.setFillColor(edgeColor);
        
        // Position weight text along the curve
        sf::Vector2f textPos = curve[segments/2].position;
        weightText.setPosition(textPos);
        
        // Add background to weight text
        sf::FloatRect textBounds = weightText.getLocalBounds();
        sf::RectangleShape textBg(sf::Vector2f(textBounds.width + 10, textBounds.height + 10));
        textBg.setPosition(textPos.x - 5, textPos.y - 5);
        textBg.setFillColor(sf::Color(255, 255, 255, 200));
        
        window.draw(textBg);
        window.draw(weightText);
    }

    // Draw vertices with animations
    for (int i = 0; i < V; i++) {
        float radius = 25;
        
        // Offset the vertex position
        sf::CircleShape vertex(radius);
        vertex.setPosition(vertexPositions[i].x + graphStartX - radius, vertexPositions[i].y + graphStartY - radius);
        
        if (i == currentU || i == currentV) {
            vertex.setFillColor(sf::Color(255, 255, 200));
        } else {
            vertex.setFillColor(sf::Color::White);
        }
        
        vertex.setOutlineColor(sf::Color::Black);
        vertex.setOutlineThickness(2);
        window.draw(vertex);

        sf::Text vertexText;
        vertexText.setFont(font);
        vertexText.setString(std::to_string(i));
        vertexText.setCharacterSize(20);
        vertexText.setFillColor(sf::Color::Black);
        
        sf::FloatRect textBounds = vertexText.getLocalBounds();
        vertexText.setPosition(
            vertexPositions[i].x + graphStartX - textBounds.width/2,
            vertexPositions[i].y + graphStartY - textBounds.height/2
        );
        
        window.draw(vertexText);
    }
}


  void drawCode() {
    float startX = 50;
    float startY = 150; // Adjust this position to ensure it doesn't overlap with the graph
    float lineHeight = 25;

    for (size_t i = 0; i < codeLines.size(); i++) {
        sf::Text lineText;
        lineText.setFont(font);
        lineText.setString(codeLines[i]);
        lineText.setCharacterSize(18);
        lineText.setPosition(startX, startY + i * lineHeight);
        
        if (i == currentLine) {
            sf::RectangleShape highlight(sf::Vector2f(700, lineHeight));
            highlight.setPosition(startX - 5, startY + i * lineHeight);
            highlight.setFillColor(sf::Color(255, 240, 240));
            window.draw(highlight);
            
            lineText.setFillColor(sf::Color::Red);
            
            // Update debug info
            std::stringstream debug;
            debug << "Current State:\n";
            if (currentLine >= 3 && currentLine <= 5) {
                debug << "Initializing distances\n";
                debug << "i = " << currentI;
            }
            else if (currentLine >= 8 && currentLine <= 16) {
                debug << "Relaxing edges\n";
                debug << "Iteration: " << currentI << "/" << (V-1) << "\n";
                debug << "Edge: " << currentJ << "/" << E << "\n";
                debug << "u = " << currentU << ", v = " << currentV << "\n";
                debug << "weight = " << currentWeight;
                if (isRelaxed) {
                    debug << "\nEdge relaxed!";
                }
            }
            
            debugInfo.setString(debug.str());
        } else {
            lineText.setFillColor(sf::Color::Black);
        }
        
        window.draw(lineText);
    }
}


    void drawControls() {
        // Draw play/pause button
        playPauseButton.setSize(sf::Vector2f(100, 40));
        playPauseButton.setPosition(50, 700);
        playPauseButton.setFillColor(isPlaying ? sf::Color::Red : sf::Color::Green);
        window.draw(playPauseButton);
        
        playPauseText.setString(isPlaying ? "Pause" : "Play");
        playPauseText.setPosition(70, 708);
        window.draw(playPauseText);
        
        // Draw speed controls
        speedUpButton.setSize(sf::Vector2f(40, 40));
        speedUpButton.setPosition(160, 700);
        speedUpButton.setFillColor(sf::Color::Blue);
        window.draw(speedUpButton);
        
        slowDownButton.setSize(sf::Vector2f(40, 40));
        slowDownButton.setPosition(210, 700);
        slowDownButton.setFillColor(sf::Color::Blue);
        window.draw(slowDownButton);
        
        // Draw speed text
        std::stringstream ss;
        ss << "Speed: " << simulationSpeed << "x";
        speedText.setString(ss.str());
        speedText.setPosition(260, 708);
        window.draw(speedText);
    }

public:
    BellmanFordSimulator(const std::vector<Edge>& inputEdges, int vertices, int edges) 
        : window(sf::VideoMode(1200, 800), "Bellman Ford Algorithm Simulator"),
          edges(inputEdges),
          V(vertices),
          E(edges),
          dist(vertices, INT_MAX),
          currentLine(0),
          currentI(0),
          currentJ(0),
          currentU(-1),
          currentV(-1),
          currentWeight(0),
          isRelaxed(false),
          isPlaying(false),
          simulationSpeed(1.0f) {
        
        window.setFramerateLimit(60);
        
        if (!font.loadFromFile("/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf")) {
            std::cerr << "Error loading font" << std::endl;
        }

        // Initialize UI elements
        playPauseText.setFont(font);
        playPauseText.setCharacterSize(20);
        playPauseText.setFillColor(sf::Color::White);
        
        speedText.setFont(font);
        speedText.setCharacterSize(20);
        speedText.setFillColor(sf::Color::Black);
        
        debugInfo.setFont(font);
        debugInfo.setCharacterSize(20);
        debugInfo.setFillColor(sf::Color::Blue);
        debugInfo.setPosition(800, 50);

        initializeCode();
        initializeVertexPositions();
    }

    void step() {
        isRelaxed = false;
        
        switch (currentLine) {
            case 0: 
            case 1: 
                currentLine++;
                break;
                
            case 2: 
            case 3: 
                currentLine++;
                currentI = 0;
                break;
                
            case 4: 
                if (currentI < V) {
                    dist[currentI] = INT_MAX;
                    currentI++;
                } else {
                    currentLine++;
                }
                break;
                
            case 5: 
                dist[0] = 0;
                currentLine++;
                break;
                
            case 6: 
            case 7: 
                currentLine++;
                currentI = 1;
                break;
                
            case 8: 
                if (currentI <= V - 1) {
                    currentJ = 0;
                    currentLine++;
                } else {
                    currentLine = 17; 
                }
                break;
                
            case 9:
                if (currentJ < E) {
                    currentU = edges[currentJ].src;
                    currentV = edges[currentJ].dest;
                    currentWeight = edges[currentJ].weight;
                    currentLine++;
                } else {
                    currentI++;
                    currentLine = 8;
                }
                break;
                
            case 10:
                if (dist[currentU] != INT_MAX && dist[currentU] + currentWeight < dist[currentV]) {
                    currentLine++;
                    isRelaxed = true;
                } else {
                    currentJ++;
                    currentLine = 9;
                }
                break;
                
            case 11:
                dist[currentV] = dist[currentU] + currentWeight;
                currentJ++;
                currentLine = 9;
                break;
                
            case 17:
            case 18:
                currentJ = 0;
                currentLine++;
                break;
                
            case 19:
                if (currentJ < E) {
                    currentU = edges[currentJ].src;
                    currentV = edges[currentJ].dest;
                    currentWeight = edges[currentJ].weight;
                    if (dist[currentU] != INT_MAX && dist[currentU] + currentWeight < dist[currentV]) {
                        currentLine = 23;  // Negative cycle detected
                    } else {
                        currentJ++;
                    }
                } else {
                    currentLine++;  // Algorithm complete
                }
                break;
        }
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    
                    // Handle play/pause button click
                    if (playPauseButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        isPlaying = !isPlaying;
                        stepClock.restart();
                    }
                    
                    // Handle speed up button click
                    if (speedUpButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        simulationSpeed = std::min(simulationSpeed * 2.0f, 8.0f);
                    }
                    
                    // Handle slow down button click
                    if (slowDownButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        simulationSpeed = std::max(simulationSpeed / 2.0f, 0.25f);
                    }
                }
            }
            
            // Handle keyboard controls
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    isPlaying = !isPlaying;
                    stepClock.restart();
                }
                else if (event.key.code == sf::Keyboard::Up) {
                    simulationSpeed = std::min(simulationSpeed * 2.0f, 8.0f);
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    simulationSpeed = std::max(simulationSpeed / 2.0f, 0.25f);
                }
            }
        }
    }

    void run() {
        stepClock.restart();
        
        while (window.isOpen()) {
            handleEvents();
            
            if (isPlaying && stepClock.getElapsedTime().asSeconds() > 1.0f / simulationSpeed) {
                step();
                stepClock.restart();
            }

            window.clear(sf::Color(245, 245, 245));  // Light gray background
            
            drawArray();
            drawGraph();
            drawCode();
            drawControls();
            
            window.draw(debugInfo);
            window.display();
        }
    }
};

int main() {
    // Example graph
    std::vector<Edge> edges = {
        {0, 1, -1}, {0, 2, 4}, {1, 2, 3}, {2, 0, 2}
    };
    
    BellmanFordSimulator simulator(edges, 3, 4);
    simulator.run();
    return 0;
}