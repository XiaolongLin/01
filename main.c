#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_ORDERS 16
#define MAX_DRIVERS 8
#define MAX_CUSTOMERS 8
#define MAX_SUPPLIERS 8
#define MAX_LEDGER 32
#define MAX_USERS 8

typedef enum {
  ORDER_CREATED,
  ORDER_IN_TRANSIT,
  ORDER_SIGNED
} OrderStatus;

typedef struct {
  const char *at;
  const char *event;
} TimelineEvent;

typedef struct {
  int id;
  int supplierId;
  int customerId;
  int driverId;
  const char *plateNo;
  const char *date;
  double tons;
  double freightPayable;
  double freightPaid;
  double purchasePayable;
  double purchasePaid;
  double salesReceivable;
  double salesReceived;
  OrderStatus status;
  TimelineEvent timeline[3];
} Order;

typedef struct {
  int id;
  const char *name;
  const char *phone;
  const char *plateNo;
  double capacityTons;
  const char *note;
} Driver;

typedef struct {
  int id;
  const char *name;
  const char *contact;
  const char *address;
  int creditDays;
  double creditLimit;
} Customer;

typedef struct {
  int id;
  const char *name;
  const char *contact;
  const char *address;
  double commonPricePerTon;
} Supplier;

typedef struct {
  int id;
  const char *date;
  const char *type;
  const char *targetType;
  int targetId;
  int orderId;
  double amount;
  const char *note;
  const char *voucher;
} LedgerEntry;

typedef struct {
  const char *username;
  const char *password;
  const char *role;
} User;

typedef struct {
  Order orders[MAX_ORDERS];
  int orderCount;
  Driver drivers[MAX_DRIVERS];
  int driverCount;
  Customer customers[MAX_CUSTOMERS];
  int customerCount;
  Supplier suppliers[MAX_SUPPLIERS];
  int supplierCount;
  LedgerEntry ledger[MAX_LEDGER];
  int ledgerCount;
  User users[MAX_USERS];
  int userCount;
} AppState;

const char *status_to_str(OrderStatus status) {
  if (status == ORDER_CREATED) return "已创建";
  if (status == ORDER_IN_TRANSIT) return "运输中";
  return "已签收";
}

const Driver *find_driver(const AppState *app, int id) {
  for (int i = 0; i < app->driverCount; i++) if (app->drivers[i].id == id) return &app->drivers[i];
  return NULL;
}

const Customer *find_customer(const AppState *app, int id) {
  for (int i = 0; i < app->customerCount; i++) if (app->customers[i].id == id) return &app->customers[i];
  return NULL;
}

const Supplier *find_supplier(const AppState *app, int id) {
  for (int i = 0; i < app->supplierCount; i++) if (app->suppliers[i].id == id) return &app->suppliers[i];
  return NULL;
}

bool login(const AppState *app, const char *username, const char *password, User *user) {
  for (int i = 0; i < app->userCount; i++) {
    if (strcmp(app->users[i].username, username) == 0 && strcmp(app->users[i].password, password) == 0) {
      *user = app->users[i];
      return true;
    }
  }
  return false;
}

void seed_data(AppState *app) {
  app->drivers[0] = (Driver){1, "王师傅", "13800000001", "豫A-12345", 35.0, "路线熟、夜间可跑"};
  app->drivers[1] = (Driver){2, "李师傅", "13800000002", "豫B-88991", 32.0, "月底结算"};
  app->driverCount = 2;

  app->customers[0] = (Customer){101, "华北饲料厂", "刘经理", "郑州高新区", 30, 300000.0};
  app->customers[1] = (Customer){102, "顺兴饲料厂", "张主管", "新乡牧野区", 20, 180000.0};
  app->customerCount = 2;

  app->suppliers[0] = (Supplier){201, "金穗面粉场", "赵总", "开封工业园", 3180.0};
  app->suppliers[1] = (Supplier){202, "丰收面粉场", "何总", "安阳开发区", 3150.0};
  app->supplierCount = 2;

  app->orders[0] = (Order){
      1001, 201, 101, 1, "豫A-12345", "2026-03-10", 35.0,
      4200.0, 2000.0,
      111300.0, 60000.0,
      122500.0, 50000.0,
      ORDER_IN_TRANSIT,
      {{"2026-03-10 08:00", "已创建"}, {"2026-03-10 10:20", "已装车发运"}, {"2026-03-11 06:30", "运输中"}}
  };
  app->orders[1] = (Order){
      1002, 202, 102, 2, "豫B-88991", "2026-03-06", 32.0,
      3900.0, 3900.0,
      100800.0, 100800.0,
      110400.0, 110400.0,
      ORDER_SIGNED,
      {{"2026-03-06 09:00", "已创建"}, {"2026-03-06 11:00", "运输中"}, {"2026-03-07 07:20", "已签收"}}
  };
  app->orderCount = 2;

  app->ledger[0] = (LedgerEntry){5001, "2026-03-07", "收款", "客户", 102, 1002, 110400.0, "订单1002货款", "收据-2301"};
  app->ledger[1] = (LedgerEntry){5002, "2026-03-08", "付款", "供应商", 202, 1002, 100800.0, "订单1002采购款", "回单-3302"};
  app->ledger[2] = (LedgerEntry){5003, "2026-03-09", "运费付款", "司机", 1, 1001, 2000.0, "订单1001预付运费", "运费凭证-771"};
  app->ledgerCount = 3;

  app->users[0] = (User){"admin", "admin123", "ADMIN"};
  app->users[1] = (User){"finance", "finance123", "FINANCE"};
  app->users[2] = (User){"ops", "ops123", "OPS"};
  app->userCount = 3;
}

void print_top_nav(void) {
  puts("\n===== 顶部导航（5个主板块）=====");
  puts("1) 订单  2) 司机  3) 饲料厂（客户）  4) 面粉场（供应商）  5) 资金");
  puts("每个板块统一：列表页 -> 详情页（中心视图）");
}

void order_center(const AppState *app, int orderId) {
  puts("\n[订单] 列表页 /api/orders");
  for (int i = 0; i < app->orderCount; i++) {
    printf("- 订单%d 状态:%s\n", app->orders[i].id, status_to_str(app->orders[i].status));
  }

  const Order *o = NULL;
  for (int i = 0; i < app->orderCount; i++) if (app->orders[i].id == orderId) o = &app->orders[i];
  if (o == NULL) return;

  const Supplier *s = find_supplier(app, o->supplierId);
  const Customer *c = find_customer(app, o->customerId);
  const Driver *d = find_driver(app, o->driverId);

  puts("[订单] 详情页（Order Center） /api/orders/:id");
  printf("基本信息: %s -> %s, 吨数 %.1f, 司机 %s, 车牌 %s, 日期 %s\n",
         s ? s->name : "-", c ? c->name : "-", o->tons, d ? d->name : "-", o->plateNo, o->date);
  printf("状态: %s\n", status_to_str(o->status));
  puts("状态时间线:");
  for (int i = 0; i < 3; i++) printf("  - %s %s\n", o->timeline[i].at, o->timeline[i].event);
  printf("运费: 应付 %.2f / 已付 %.2f / 未付 %.2f\n", o->freightPayable, o->freightPaid, o->freightPayable - o->freightPaid);
  printf("采购款: 应付供应商 %.2f, 是否已付: %s\n", o->purchasePayable, o->purchasePayable <= o->purchasePaid ? "是" : "否");
  printf("销售款: 客户应收 %.2f, 是否已收: %s\n", o->salesReceivable, o->salesReceivable <= o->salesReceived ? "是" : "否");
  puts("附件: 回单/磅单（第二阶段）");
}

void driver_center(const AppState *app, int driverId) {
  puts("\n[司机] 列表页 /api/drivers");
  for (int i = 0; i < app->driverCount; i++) {
    printf("- 司机%d %s\n", app->drivers[i].id, app->drivers[i].name);
  }

  const Driver *d = find_driver(app, driverId);
  if (d == NULL) return;

  puts("[司机] 详情页（Driver Center） /api/drivers/:id");
  printf("司机信息: 电话 %s, 车牌 %s, 吨位 %.1f, 备注 %s\n", d->phone, d->plateNo, d->capacityTons, d->note);

  double payable = 0.0, paid = 0.0;
  puts("历史合作订单（近3月）:");
  for (int i = 0; i < app->orderCount; i++) {
    const Order *o = &app->orders[i];
    if (o->driverId != driverId) continue;
    payable += o->freightPayable;
    paid += o->freightPaid;
    printf("  - 订单%d %s 运费应付%.2f\n", o->id, o->date, o->freightPayable);
  }
  printf("运费结算汇总: 总应付 %.2f / 已付 %.2f / 未结 %.2f\n", payable, paid, payable - paid);

  puts("当前进行中订单:");
  bool hasActive = false;
  for (int i = 0; i < app->orderCount; i++) {
    if (app->orders[i].driverId == driverId && app->orders[i].status == ORDER_IN_TRANSIT) {
      printf("  - 订单%d 状态 %s\n", app->orders[i].id, status_to_str(app->orders[i].status));
      hasActive = true;
    }
  }
  if (!hasActive) puts("  - 无");
}

void customer_center(const AppState *app, int customerId) {
  puts("\n[饲料厂] 列表页 /api/customers");
  for (int i = 0; i < app->customerCount; i++) printf("- 客户%d %s\n", app->customers[i].id, app->customers[i].name);

  const Customer *c = find_customer(app, customerId);
  if (c == NULL) return;

  puts("[饲料厂] 详情页（客户中心） /api/customers/:id");
  printf("客户信息: 联系人 %s, 地址 %s, 账期 %d天, 信用额度 %.2f\n", c->contact, c->address, c->creditDays, c->creditLimit);

  double receivable = 0.0, received = 0.0;
  const char *latestShip = "-", *latestPay = "-";
  puts("关联订单列表:");
  for (int i = 0; i < app->orderCount; i++) {
    const Order *o = &app->orders[i];
    if (o->customerId != customerId) continue;
    receivable += o->salesReceivable;
    received += o->salesReceived;
    latestShip = o->date;
    printf("  - 订单%d %s 状态%s\n", o->id, o->date, status_to_str(o->status));
  }
  for (int i = 0; i < app->ledgerCount; i++) {
    if (strcmp(app->ledger[i].type, "收款") == 0 && app->ledger[i].targetId == customerId) latestPay = app->ledger[i].date;
  }
  printf("应收账款: 应收总额 %.2f / 已收 %.2f / 未收 %.2f / 逾期 %.2f\n", receivable, received, receivable - received, 0.0);
  printf("最近一次发货: %s, 最近一次收款: %s\n", latestShip, latestPay);
  puts("常用司机/常用面粉场: 王师傅 / 金穗面粉场（历史订单统计）");
}

void supplier_center(const AppState *app, int supplierId) {
  puts("\n[面粉场] 列表页 /api/suppliers");
  for (int i = 0; i < app->supplierCount; i++) printf("- 供应商%d %s\n", app->suppliers[i].id, app->suppliers[i].name);

  const Supplier *s = find_supplier(app, supplierId);
  if (s == NULL) return;

  puts("[面粉场] 详情页（供应商中心） /api/suppliers/:id");
  printf("供应商信息: 联系人 %s, 地址 %s, 常见采购价 %.2f/吨\n", s->contact, s->address, s->commonPricePerTon);

  double payable = 0.0, paid = 0.0;
  const char *latestBuy = "-", *latestPay = "-";
  puts("采购订单列表:");
  for (int i = 0; i < app->orderCount; i++) {
    const Order *o = &app->orders[i];
    if (o->supplierId != supplierId) continue;
    payable += o->purchasePayable;
    paid += o->purchasePaid;
    latestBuy = o->date;
    printf("  - 订单%d %s 状态%s\n", o->id, o->date, status_to_str(o->status));
  }
  for (int i = 0; i < app->ledgerCount; i++) {
    if (strcmp(app->ledger[i].type, "付款") == 0 && app->ledger[i].targetId == supplierId) latestPay = app->ledger[i].date;
  }
  printf("应付账款: 应付总额 %.2f / 已付 %.2f / 未付 %.2f / 逾期 %.2f\n", payable, paid, payable - paid, 0.0);
  printf("最近一次采购: %s, 最近一次付款: %s\n", latestBuy, latestPay);
  puts("常配套客户: 华北饲料厂（由订单统计）");
}

void finance_center(const AppState *app) {
  puts("\n[资金] 总览（Dashboard） /api/finance/dashboard");
  double monthReceivable = 0.0, monthReceived = 0.0;
  double monthPayable = 0.0, monthPaid = 0.0;
  double freightUnpaid = 0.0;

  for (int i = 0; i < app->orderCount; i++) {
    const Order *o = &app->orders[i];
    monthReceivable += o->salesReceivable;
    monthReceived += o->salesReceived;
    monthPayable += o->purchasePayable + o->freightPayable;
    monthPaid += o->purchasePaid + o->freightPaid;
    freightUnpaid += o->freightPayable - o->freightPaid;
  }
  double grossProfit = monthReceivable - monthPayable;
  printf("本月应收/已收/未收: %.2f / %.2f / %.2f\n", monthReceivable, monthReceived, monthReceivable - monthReceived);
  printf("本月应付/已付/未付: %.2f / %.2f / %.2f\n", monthPayable, monthPaid, monthPayable - monthPaid);
  printf("本月毛利: %.2f\n", grossProfit);
  puts("逾期应收/逾期应付: 0.00 / 0.00（示例数据）");
  printf("未结运费总额: %.2f\n", freightUnpaid);

  puts("[资金] 流水（Ledger） /api/finance/ledger");
  puts("字段: 日期 | 类型 | 对象 | 关联订单号 | 金额 | 备注/凭证");
  for (int i = 0; i < app->ledgerCount; i++) {
    const LedgerEntry *e = &app->ledger[i];
    printf("- %s | %s | %s#%d | %d | %.2f | %s (%s)\n",
           e->date, e->type, e->targetType, e->targetId, e->orderId, e->amount, e->note, e->voucher);
  }
}

int main(void) {
  AppState app = {0};
  seed_data(&app);

  User current = {0};
  if (!login(&app, "admin", "admin123", &current)) {
    puts("POST /api/login => {\"error\":\"unauthorized\"}");
    return 1;
  }

  printf("POST /api/login => {\"user\":\"%s\",\"role\":\"%s\"}\n", current.username, current.role);
  print_top_nav();
  order_center(&app, 1001);
  driver_center(&app, 1);
  customer_center(&app, 101);
  supplier_center(&app, 201);
  finance_center(&app);
  return 0;
}
