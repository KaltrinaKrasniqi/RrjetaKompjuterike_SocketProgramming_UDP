UDP Server-Client Protocol
Ky projekt implementon një sistem komunikimi server-klient duke përdorur protokollin UDP në gjuhën C për një rrjet me të paktën 4 pajisje reale.
Funksionalitete
Serveri
* Konfigurim: Vendos numrin e portit dhe adresën IP.
* Menaxhimi i Lidhjeve: Pranon lidhje nga klientët dhe kontrollon kufirin e lidhjeve.
* Logim & Audit: Logon të gjitha kërkesat dhe ruan mesazhet e klientëve.
* Qasje në File: Jep qasje në përmbajtjen e dosjeve për klientët e autorizuar.
Klienti
* Krijimi i Socket-it: Çdo klient lidhet me serverin përmes një socket-i.
* Privilegje: Një klient ka privilegje të plota (write, read, execute), ndërsa të tjerët vetëm read.
* Dërgim & Pranim Mesazhesh: Klientët dërgojnë dhe marrin mesazhe nga serveri.
Kërkesat
* Gjuhë Programimi: C
* Protokolli: UDP
* Numri i Pajisjeve: 4
