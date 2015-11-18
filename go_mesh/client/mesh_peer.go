// TJ DeVries
// CS 374: HPC
// Golang distributed computing
// This was adapted from the solution to part 9 of the Whispering Gophers code lab.
//
//
// It connects to the peer specified by -peer.
// It accepts connections from peers and receives messages from them.
// When it sees a peer with an address it hasn't seen before, it makes a
// connection to that peer.
// It adds an ID field containing a random string to each outgoing message.
// When it recevies a message with an ID it hasn't seen before, it broadcasts
// that message to all connected peers.
//
package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"sync"

	"code.google.com/p/whispering-gophers/util"
)

// Parse the peer address variable
var (
	hostAddr = flag.String("peer", "", "peer host:port")
	self     string
)

// Declare a Client struct, this will help us identify information about each client
type Client struct {
    ID int
    Addr string
    IsHost bool
    HostFound bool
}

func (self *Client) SetDefaults() {
    self.ID = -1
    self.Addr = ""
    self.IsHost = false
    self.HostFound = false
}

// Declare our Message struct, which will get passed from each of the peers to the host.
type Message struct {
	ID   string
	Addr string
	Body string
}

func main() {
    // Parse the items sent in by command line
	flag.Parse()

    var self_client Client
    self_client.SetDefaults()

    // Debug print statement to show that we have initialize the client correctly
    fmt.Println("Self ID is", self_client.ID, ", Addr is:", self_client.Addr, ", Hostfound is:", self_client.HostFound)

    // Get our IP address
	l, err := util.Listen()
	if err != nil {
		log.Fatal(err)
	}
	self_client.Addr = l.Addr().String()

    fmt.Println("Self ID is", self_client.ID, ", Addr is:", self_client.Addr, ", Hostfound is:", self_client.HostFound)

	// Dial the address of the host
    go dial(*hostAddr)

    // Make sure that it is the host
    go checkIfHost()

    // Send something to the host here

    // Read something back from the host here
	go readInput()

	for {
		c, err := l.Accept()
		if err != nil {
			log.Fatal(err)
		}
		go serve(c)
	}
}

// The actual variable that holds the peers we have found
// make(map[string]chan<-Message) means we are making a hash table of strings, which only accept Message into it
var peers = &Peers{m: make(map[string]chan<- Message)}

type Peers struct {
	m  map[string]chan<- Message
	mu sync.RWMutex
}

// Add creates and returns a new channel for the given peer address.
// If an address already exists in the registry, it returns nil.
func (p *Peers) Add(addr string) <-chan Message {
	p.mu.Lock()
	defer p.mu.Unlock()
	if _, ok := p.m[addr]; ok {
		return nil
	}
	ch := make(chan Message)
	p.m[addr] = ch
	return ch
}

// Remove deletes the specified peer from the registry.
//  This is a struct specific method
func (p *Peers) Remove(addr string) {
	p.mu.Lock()
    // Ensure that we unlock the peers mutex
	defer p.mu.Unlock()
    // Attempt to delete the address from the peer message map
	delete(p.m, addr)
}

// List returns a slice of all active peer channels.
func (p *Peers) List() []chan<- Message {
	p.mu.RLock()
	defer p.mu.RUnlock()
	l := make([]chan<- Message, 0, len(p.m))
	for _, ch := range p.m {
		l = append(l, ch)
	}
	return l
}

// Message everything that has been seen so far a message.
func broadcast(m Message) {
	for _, ch := range peers.List() {
		select {
		case ch <- m:
		default:
			// Okay to drop messages sometimes.
		}
	}
}

func serve(c net.Conn) {
	defer c.Close()
	d := json.NewDecoder(c)
	for {
		var m Message
		err := d.Decode(&m)
		if err != nil {
			log.Println(err)
			return
		}
		if Seen(m.ID) {
			continue
		}
		fmt.Printf("%#v\n", m)
		broadcast(m)
		go dial(m.Addr)
	}
}

// See if the connection we've made is to the host
func checkIfHost() {
    
}

// Read the information we got from our TCP connection
func readInput() {
    // Create a new scanner
	s := bufio.NewScanner(os.Stdin)

    // While we are scanning the message
	for s.Scan() {
        // Initialize our message struct, getting the information out of it
		m := Message{
			ID:   util.RandomID(),
			Addr: self,
			Body: s.Text(),
		}
        // Add the ID of the message to a list of seen peers
		Seen(m.ID)

        // Broadcast the message out to all of the other peers
		broadcast(m)
	}
	if err := s.Err(); err != nil {
		log.Fatal(err)
	}
}

func dial(addr string) {
	if addr == self {
		return // Don't try to dial self.
	}

	ch := peers.Add(addr)
	if ch == nil {
		return // Peer already connected.
	}
	defer peers.Remove(addr)

	c, err := net.Dial("tcp", addr)
	if err != nil {
		log.Println(addr, err)
		return
	}
	defer c.Close()

	e := json.NewEncoder(c)
	for m := range ch {
		err := e.Encode(m)
		if err != nil {
			log.Println(addr, err)
			return
		}
	}
}

var seenIDs = struct {
	m map[string]bool
	sync.Mutex
}{m: make(map[string]bool)}

// Seen returns true if the specified id has been seen before.
// If not, it returns false and marks the given id as "seen".
func Seen(id string) bool {
	seenIDs.Lock()
	ok := seenIDs.m[id]
	seenIDs.m[id] = true
	seenIDs.Unlock()
	return ok
}
