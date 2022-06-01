TOTAL_SIMULATION_TIME = 300  # [s]


class Measurement:
    CASE_TAG = "Case"
    ITV_TAG = "Generation Interval"
    GEN_TAG = "Packets Generated"
    DELIVERED_TAG = "Packets Delivered"
    DROPQ_TAG = "Dropped on Queue"
    DROPR_TAG = "Dropped on Receiver"
    TOTALDROP_TAG = "Dropped Packets"
    AVDEL_TAG = "Average Delay"
    GENRATE_TAG = "Generation Rate"
    RECRATE_TAG = "Reception Rate"
    DROPRATE_TAG = "Drop Rate"
    RTT_TAG = "RTT"
    ACKT_TAG = "ACK Time"
    RET_TAG = "Amount Retransmitted"

    def __init__(self, case, itv, gen, delivered, dropQ, dropR, avdel, rtt=0, ackt=0, ret=0):
        self.case = case
        self.itv = float(itv)
        self.gen = int(gen)
        self.delivered = int(delivered)
        self.dropQ = int(dropQ)
        self.dropR = int(dropR)
        self.total_drop = self.dropQ + self.dropR
        self.avdel = float(avdel)
        self.gen_per_second = self.gen / TOTAL_SIMULATION_TIME
        self.rec_per_second = self.delivered / TOTAL_SIMULATION_TIME
        self.drop_rate = 1 - self.delivered/self.gen
        self.rtt = rtt
        self.ackt = ackt
        self.ret = ret

    def __str__(self) -> str:
        return str(self.to_dictionary())

    def to_dictionary(self) -> dict:
        return {
            self.CASE_TAG: self.case,
            self.ITV_TAG: self.itv,
            self.GEN_TAG: self.gen,
            self.DELIVERED_TAG: self.delivered,
            self.DROPQ_TAG: self.dropQ,
            self.DROPR_TAG: self.dropR,
            self.AVDEL_TAG: self.avdel,
            self.GENRATE_TAG: self.gen_per_second,
            self.RECRATE_TAG: self.rec_per_second,
            self.DROPRATE_TAG: self.drop_rate,
            self.RTT_TAG: self.rtt,
            self.ACKT_TAG: self.ackt,
            self.RET_TAG: self.ret
        }

    @staticmethod
    def parse(line: str, case: str):
        """
        Parses a line like `| 2.0 | 144 | 144 | 0 | 0 | 0.40 |`
        to a Measurement instance
        """
        line = "".join(list(filter(lambda ch: ch != " ", line))).strip()
        args = line.split("|")[1:-1]
        args.insert(0, case)
        return Measurement(*args)
